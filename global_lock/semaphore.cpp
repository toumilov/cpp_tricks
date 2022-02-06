
#include <semaphore.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

class GlobalLock
{
public:
	GlobalLock( const char *name ) :
		locked_( false )
	{
		errno = 0;
		// Initialize to 1 (unlocked)
		sem_ = sem_open( name, O_CREAT, 0644, 1 );
		if ( sem_ != SEM_FAILED && errno == ENOENT )
		{
			// Semaphore created, mark it for cleanup.
			name_ = std::string( name );
		}
	}

	~GlobalLock()
	{
		if ( operator bool() )
		{
			unlock();
			sem_close( sem_ );
			if ( !name_.empty() )
			{
				sem_unlink( name_.c_str() );
			}
		}
	}

	operator bool() const
	{
		return sem_ != SEM_FAILED;
	}

	bool locked() const
	{
		if ( operator bool() )
		{
			if ( locked_ )
			{
				return true;
			}
			int val = 0;
			if ( sem_getvalue( sem_, &val ) == 0 )
			{
				return val <= 0;
			}
		}
		return false;
	}

	bool lock()
	{
		if ( !operator bool() || locked_ )
		{
			return false;
		}
		errno = 0;
		locked_ = sem_wait( sem_ ) == 0;
		return locked_;
	}

	bool try_lock()
	{
		if ( !operator bool() || locked_ )
		{
			return false;
		}
		locked_ = sem_trywait( sem_ ) == 0;
		return locked_;
	}

	bool try_lock( unsigned millisec )
	{
		if ( !operator bool() || locked_ )
		{
			return false;
		}
		struct timespec ts;
		if ( clock_gettime( CLOCK_REALTIME, &ts ) >= 0 )
		{
			unsigned reminder = millisec % 1000;
			ts.tv_sec += ( millisec / 1000 );
			ts.tv_nsec += ( reminder * 1000000 );
			locked_ = sem_timedwait( sem_, &ts ) == 0;
		} else {
			// If failed to get current time, try to lock immediately
			locked_ = sem_trywait( sem_ ) == 0;
		}
		return locked_;
	}

	bool unlock()
	{
		if ( !operator bool() || !locked_ )
		{
			return false;
		}
		locked_ = false;
		return sem_post( sem_ ) == 0;
	}

private:
	sem_t *sem_;
	bool locked_;
	std::string name_;
};

class GlobalScopedLock
{
public:
	GlobalScopedLock( const char *name ) :
		lock_( name )
	{
		lock_.lock();
	}

	~GlobalScopedLock()
	{
		lock_.unlock();
	}

	operator bool() const
	{
		return lock_;
	}

private:
	GlobalLock lock_;
};


int main()
{
	{
		// Scoped lock test
		GlobalScopedLock lck( "/test" );
		printf( "Working...\n" );
		sleep( 3 );
		printf( "Done scoped lock\n" );
	}

	GlobalLock lck( "/test" );
	if ( !lck )
	{
		printf( "Failed to create lock\n" );
		return 1;
	}
	if ( lck.try_lock() )
	{
		printf( "Working...\n" );
		sleep( 3 );
	} else {
		printf( "Already locked\n" );
		if ( lck.try_lock( 3000 ) )
		{
			if ( lck.locked() )
			{
				printf( "Working...\n" );
				sleep( 3 );
				lck.unlock();
			}
		}
	}
	printf( "Done\n" );

	return 0;
}
