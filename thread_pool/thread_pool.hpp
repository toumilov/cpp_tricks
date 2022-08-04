
#include <thread>
#include <mutex>
#include <vector>


class ThreadPool
{
public:
	struct Worker
	{
		Worker();
		~Worker();

		Worker( const Worker& ) = delete;
		Worker( const Worker&& ) = delete;
		Worker& operator=( const Worker& ) = delete;
		Worker& operator=( const Worker&& ) = delete;

		template <class Fn, class... Args>
		void run( Fn &&fn, Args &&... args )
		{
			mtx_.lock();
			started_ = true;
			thread_ = std::thread( &Worker::wrapper<Fn, Args...>, this,
					std::forward<Fn>( fn ), std::forward<Args>( args )... );
			mtx_.unlock();
		}

	private:
		friend class ThreadPool;
		std::thread thread_;
		std::mutex mtx_;
		bool started_;
		bool done_;

		template <class Fn, class... Args>
		void wrapper( Fn &&fn, Args &&... args )
		{
			fn( args... );
			mtx_.lock();
			done_ = true;
			mtx_.unlock();
		}

		operator bool();
		void swap( Worker &rhs );
		void join();
	};

	ThreadPool( int size );

	Worker& get();
	void join();

private:
	std::vector<Worker> workers_;
	std::mutex mtx_;
	bool joining_;
};
