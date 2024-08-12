#include "thread_pool.hpp"

ThreadPool::Worker::Worker() :
	started_( false ),
	done_( false )
{}

ThreadPool::Worker::~Worker()
{
	join();
}

ThreadPool::Worker::operator bool()
{
	mtx_.lock();
	bool ret = !started_ || ( started_ && done_ );
	mtx_.unlock();
	return ret;
}

void ThreadPool::Worker::swap( Worker &rhs )
{
	thread_.swap( rhs.thread_ );
	std::swap( started_, rhs.started_ );
	std::swap( done_, rhs.done_ );
}

void ThreadPool::Worker::join()
{
	while( true )
	{
		mtx_.lock();
		if ( !started_ )
		{
			mtx_.unlock();
			break;
		}
		if ( done_ )
		{
			thread_.join();
			started_ = false;
			done_ = false;
			mtx_.unlock();
			break;
		}
		mtx_.unlock();
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
	}
}

ThreadPool::ThreadPool( int size ) :
	workers_( size ),
	joining_( false )
{
}

ThreadPool::Worker& ThreadPool::get()
{
	while( true )
	{
		{
			std::lock_guard<std::mutex> lck( mtx_ );
			for( auto &w : workers_ )
			{
				if ( w )
				{
					Worker tmp;
					w.swap( tmp );
					return w;
				}
			}
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
	}
}

void ThreadPool::join()
{
	mtx_.lock();
	if ( !joining_ )
	{
		joining_ = true;
		for( auto &w : workers_ )
		{
			w.join();
		}
	}
	mtx_.unlock();
}