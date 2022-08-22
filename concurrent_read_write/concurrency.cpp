
#include <cstdio>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <mutex>
#include <functional>
#include <vector>
#include <utility>


#define READ_TIME 1
#define WRITE_TIME 2


template<typename T>
class ConcurrentContainer
{
public:
	ConcurrentContainer( const T &value ) :
		data_( value ),
		readers_( 0 ),
		write_flag_( false )
	{}

	T get()
	{
		ScopedReadLock guard( *this );
		std::this_thread::sleep_for( std::chrono::milliseconds( READ_TIME ) );
		return data_;
	}

	void set( const T &value )
	{
		ScopedWriteLock guard( *this );
		std::this_thread::sleep_for( std::chrono::milliseconds( WRITE_TIME ) );
		data_ = value;
	}

private:
	T data_;
	std::atomic<int> readers_;
	std::atomic<bool> write_flag_;

	struct ScopedReadLock
	{
		ConcurrentContainer &container;

		ScopedReadLock( ConcurrentContainer &c ) :
			container( c )
		{
			container.acquire_read();
		}
		~ScopedReadLock()
		{
			container.release_read();
		}
	};

	struct ScopedWriteLock
	{
		ConcurrentContainer &container;

		ScopedWriteLock( ConcurrentContainer &c ) :
			container( c )
		{
			container.acquire_write();
		}
		~ScopedWriteLock()
		{
			container.release_write();
		}
	};

	inline void acquire_read()
	{
		// Wait until write operation is complete
		while( write_flag_.load() == true )
		{
			std::this_thread::yield();
		}
		// Increment readers
		readers_++;
	}

	inline void release_read()
	{
		// Decrease readers
		readers_--;
	}

	inline void acquire_write()
	{
		// Wait until write operation is complete
		bool expected = false;
		while( !write_flag_.compare_exchange_strong( expected, true ) )
		{
			std::this_thread::yield();
			expected = false;
		}
		// Wait until all read operations are complete
		while( readers_.load() > 0 )
		{
			std::this_thread::yield();
		}
	}

	inline void release_write()
	{
		write_flag_.store( false );
	}
};

template<typename T>
class MutexContainer
{
public:
	MutexContainer( const T &value ) :
		data_( value )
	{}

	T get()
	{
		std::lock_guard<std::mutex> lck( mtx_ );
		std::this_thread::sleep_for( std::chrono::milliseconds( READ_TIME ) );
		return data_;
	}

	void set( const T &value )
	{
		std::lock_guard<std::mutex> lck( mtx_ );
		std::this_thread::sleep_for( std::chrono::milliseconds( WRITE_TIME ) );
		data_ = value;
	}

private:
	T data_;
	std::mutex mtx_;
};


int main()
{
	std::vector<std::thread> threads;

	// Compare mutex lock vs concurrent container
	int iterations = 1000;

	MutexContainer<int> mc( 5 );
	auto start = std::chrono::steady_clock::now();
	auto foo = [&](){
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution( 0, 9 );
		int last = 0;
		for( int i = 0; i < iterations; ++i )
		{
			int number = distribution( generator );
			if ( number <= 2 ) // 10% - write
			{
				mc.set( last );
				last++;
			} else {
				last = mc.get();
			}
		}
	};
	threads.push_back( std::thread( foo ) );
	threads.push_back( std::thread( foo ) );
	for( auto &th : threads )
	{
		th.join();
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	printf( "Mutex container reached %d in %f seconds\n", mc.get(), elapsed_seconds.count() );
	threads.clear();

	ConcurrentContainer<int> cc( 5 );
	start = std::chrono::steady_clock::now();
	auto foo2 = [&](){
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution( 0, 9 );
		int last = 0;
		for( int i = 0; i < iterations; ++i )
		{
			int number = distribution( generator );
			if ( number <= 2 ) // 10% - write
			{
				cc.set( last );
				last++;
			} else {
				last = cc.get();
			}
		}
	};
	threads.push_back( std::thread( foo2 ) );
	threads.push_back( std::thread( foo2 ) );
	for( auto &th : threads )
	{
		th.join();
	}
	end = std::chrono::steady_clock::now();
	elapsed_seconds = end-start;
	printf( "Concurrent container reached %d in %f seconds\n", cc.get(), elapsed_seconds.count() );
	threads.clear();
}
