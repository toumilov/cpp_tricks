
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <vector>


class Subprocess
{
public:
	enum class Status
	{
		Ok,
		Timeout,
		Failed,
		Closed
	};
	static const uid_t InvalidUid = (uid_t)-1;
	static const gid_t InvalidGid = (gid_t)-1;

	static uid_t get_uid( const std::string &user_name )
	{
		struct passwd *pwd = getpwnam( user_name.c_str() );
		if ( pwd == NULL )
		{
			return InvalidUid;
		}
		return pwd->pw_uid;
	}

	static gid_t get_gid( const std::string &group_name )
	{
		struct group *grp = getgrnam( group_name.c_str() );
		if ( grp == NULL )
		{
			return InvalidGid;
		}
		return grp->gr_gid;
	}

	Subprocess( const char *cmd ) :
		pid_( 0 ),
		path_( cmd )
	{
		auto pos = path_.rfind( '/' );
		if ( pos != std::string::npos )
		{
			cmd_ = path_.substr( pos + 1 );
		}
		for( auto &i : io_ )
		{
			close( i );
			i = 0;
		}
		sigpipe_handler_.sa_handler = SIG_IGN;
		sigemptyset( &sigpipe_handler_.sa_mask );
		sigpipe_handler_.sa_flags = 0;
		sigaction( SIGPIPE, &sigpipe_handler_, 0 );
		if ( sigaction( SIGPIPE, NULL, &sigpipe_handler_ ) != 0 )
		{
			sigpipe_handler_.sa_handler = SIG_DFL;
		}
		signal( SIGPIPE, SIG_IGN );
	}

	~Subprocess()
	{
		signal( SIGPIPE, sigpipe_handler_.sa_handler );
		for( auto &i : io_ )
		{
			close( i );
			i = 0;
		}
	}

	int wait_pid()
	{
		int status = 0;
		waitpid( pid_, &status, 0 );
		pid_ = 0;
		return WEXITSTATUS( status );
	}

	bool run( uid_t uid = InvalidUid, gid_t gid = InvalidGid, const char **env = NULL )
	{
		if ( pid_ )
		{
			return false;
		}
		bool ret = true;
		do
		{
			for( auto &i : io_ )
			{
				close( i );
				i = 0;
			}
			if ( pipe( &io_[ParentIn] ) || pipe( &io_[ChildIn] ) )
			{
				ret = false;
				break;
			}
			fflush( 0 );
			pid_ = fork();
			switch( pid_ )
			{
				case -1:
					ret = false;
					break;
				case 0: // Child
					if ( dup2( io_[ParentIn], STDIN_FILENO ) == -1 )
					{
						fprintf(stderr, "dup1 failed[%d]\n", errno);fflush(0);
					}
					if ( dup2( io_[ChildOut], STDOUT_FILENO ) == -1 )
					{
						fprintf(stderr, "dup2 failed[%d]\n", errno);fflush(0);
					}
					close( io_[ParentOut]); // Close unused write end
					close( io_[ChildIn]);   // Close unused read end
					if ( gid != InvalidGid )
					{
						setuid( gid );
					}
					if ( uid != InvalidUid )
					{
						setuid( uid );
					}
					if ( env )
					{
						do
						{
							putenv( (char*)*env );
							env++;
						} while( env );
					}
					prctl( PR_SET_PDEATHSIG, SIGTERM ); // Kill subprocess if parent dies
					execl( path_.c_str(), cmd_.c_str(), NULL );
					exit(1); // Should never get here
					break;
				default: // Parent
					close(io_[ParentIn]); // Close unused read end
					close(io_[ChildOut]); // Close unused write end
					break;
			}
		} while( false );
		if ( !ret )
		{
			for( auto &i : io_ )
			{
				close( i );
				i = 0;
			}
			pid_ = 0;
		}
		return ret;
	}

	Status read( std::string &out, unsigned timeout_sec = 3 ) const
	{
		const int &fd = io_[ChildIn];
		fd_set fds;
		struct timeval timeout;

		timeout.tv_sec = timeout_sec;
		timeout.tv_usec = 0;

		FD_ZERO( &fds );
		FD_SET( fd, &fds );

		Status status = Status::Ok;
		errno = 0;
		int i = select( fd + 1, &fds, (fd_set*)NULL, (fd_set*)NULL, &timeout );
		switch( i )
		{
			case 0: /* Timeout expired */
				status = Status::Timeout;
				fprintf(stderr, "READ timeout\n");
				break;
			case -1:
				status = Status::Failed;
				fprintf(stderr, "READ:%d\n", errno);
				break;
			default:
			{
				std::vector<char> buf( 100 );
				bool done = false;
				while( !done )
				{
					ssize_t bytes = ::read( fd, (void*)buf.data(), buf.size() );
					switch( bytes )
					{
						case -1:
							status = Status::Failed;
							/*fallthrough*/
						case 0:
							status = Status::Closed;
							done = true;
							break;
						default:
							out.append( buf.data(), bytes );
							if ( bytes < buf.size() )
							{
								done = true;
							}
							break;
					}
				}
				break;
			}
		}
		return status;
	}

	Status expect( const std::string &text ) const
	{
		std::string out;
		do
		{
			auto s = read( out );
			if ( s != Subprocess::Status::Ok )
			{
				fprintf(stderr, "READ: %d\n", (int)s);
				return s;
			}
		} while( out.find( text ) == std::string::npos );
		return Subprocess::Status::Ok;
	}

	Status write( const std::string &cmd ) const
	{
		Status status = Status::Ok;
		size_t bytes = 0;
		size_t bytes_left = cmd.size();
		do
		{
			errno = 0;
			size_t bytes_written = ::write( io_[ParentOut], (const void*)( &cmd[bytes] ), bytes_left );
			switch( bytes_written )
			{
				case -1:
					status = ( errno == EPIPE ) ? Status::Closed : Status::Failed;
					bytes_left = 0;
					break;
				case 0:
					usleep( 100000 );
					break;
				default:
					bytes += bytes_written;
					bytes_left -= bytes_written;
					break;
			}
		} while( bytes_left > 0 );
		return status;
	}

	void purge()
	{
		std::string out;
		read( out, 1 );
	}

private:
	enum PipeFds
	{
		ParentIn = 0,
		ParentOut,
		ChildIn,
		ChildOut
	};
	pid_t pid_;
	int io_[4];
	std::string cmd_;
	std::string path_;
	struct sigaction sigpipe_handler_;
};

int main()
{
	/*const char *env[] = {
		"HOME=/home/user",
		NULL
	};*/
	Subprocess p( "./app" );
//	if ( !p.run( p.get_uid( "user" ), p.get_uid( "group" ), env ) )
	if ( !p.run() )
	{
		return 1;
	}
	if ( p.expect( "Enter command:" ) != Subprocess::Status::Ok )
	{
		return 2;
	}
	p.write( "in\n" );
	if ( p.expect( "Enter text:" ) != Subprocess::Status::Ok )
	{
		return 3;
	}
	p.write( "test string\n" );
	p.purge();
	p.write( "out\n" );
	std::string out;
	auto s = p.read( out );
	if ( s == Subprocess::Status::Ok )
	{
		printf( "{%s}\n", out.c_str() );
	} else {
		printf( "[%d]\n", (int)s );
	}
	p.write( "quit\n" );
	out.clear();
	s = p.read( out );
	if ( s == Subprocess::Status::Ok )
	{
		printf( "{%s}\n", out.c_str() );
	} else {
		printf( "[%d]\n", (int)s );
	}
	printf( "Return code: %d\n", p.wait_pid() );
	return 0;
}
