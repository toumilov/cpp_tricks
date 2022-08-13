# Concurrent read/write

Ususally, to avoid data races, programmers are using mutextes.
Essentially, mutex implementation looks like following:
```
class Mutex
{
public:
	Mutex() :
		lock_( false )
	{}
	Mutex( const Mutex &rhs ) :
		lock_( rhs.lock_.load() )
	{}
	Mutex( const Mutex &&rhs ) :
		lock_( rhs.lock_.load() )
	{}
	Mutex& operator=( Mutex &rhs )
	{
		lock_.store( rhs.lock_.load() );
		return *this;
	}
	Mutex& operator=( Mutex &&rhs )
	{
		lock_.store( rhs.lock_.load() );
		return *this;
	}

	void lock()
	{
		bool expected = false;
		while( !lock_.compare_exchange_strong( expected, true ) )
		{
			std::this_thread::yield();
			expected = false;
		}
	}

	void unlock()
	{
		lock_.store( false );
	}

private:
	std::atomic<bool> lock_;
};
```
However, in most cases read operations can be performed in parallel. Only write's must be atomic.
In this sample we have a concurrent container that implements the above idea.

# Implementation
The abstract *ConcurrentContainer* class has two atomics:
* atomic integer (readers counter), which holds the number os active readers
* atomic boolean (write lock), which indicate that write operation is currently ongoing

If read operation is requested, we first check the write lock, and if it is not locked, we increment a readers counter.
If write operation is requested, we set the write lock and wait until all read operations are complete.
By this small trick we may slightly improve performance for concurrent containers that are getting read much more often than they are modified.
