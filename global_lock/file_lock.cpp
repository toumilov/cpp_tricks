
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <string>

class GlobalLock
{
public:
	GlobalLock( const char *path ) :
		fd_( 0 ),
		path_( path )
	{
	}

	~GlobalLock()
	{
		release();
	}

	bool acquire()
	{
		if ( fd_ )
		{
			return false; // Already acquired
		}
		fd_ = open( path_.c_str(), O_CREAT | O_RDONLY, 0644 );
		return !( fd_ <= 0 || flock( fd_, LOCK_EX | LOCK_NB ) != 0 );
	}

	void release()
	{
		if ( fd_ > 0 )
		{
			flock( fd_, LOCK_UN | LOCK_NB );
			close( fd_ );
			fd_ = 0;
		}
	}

	bool locked() const
	{
		return fd_ > 0;
	}

private:
	int fd_;
	std::string path_;
};

int main()
{
	GlobalLock lock( "/tmp/test.lock" );
	if ( lock.acquire() )
	{
		printf( "Lock acquired\n" );
		printf( "Working...\n" );
		sleep( 3 );
		printf( "Done\n" );
	} else {
		printf( "Lock failed\n" );
	}
	lock.release();
}
