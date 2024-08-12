#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include "socket.hpp"
#include "thread_pool.hpp"
#include "session.hpp"
#include "proxy.hpp"

// Private proxy part, which is not supposed to be visible from the interface
struct Proxy::Private
{
	uint16_t client_port, server_port;	// Server port
	std::string server_ip;				// Server address
	std::atomic_bool stop;				// Proxy is stopping
	std::atomic_bool new_connection;	// Pending new connection flag
	TcpSocket listener;					// Listener socket
	ThreadPool pool;					// Processing threads
	bool worker_on_hold;				// Worker stop flag
	std::thread worker_thread;			// Worker thread object
	std::mutex session_mtx;				// Session list mutex
	std::condition_variable cv;			// Conditiona variable for listener and worker synchronization
	std::vector<std::shared_ptr<Session>> sessions; // Session list
	LoggerBase &logger;					// Logger reference

	Private( uint16_t client_port, const std::string &server_ip, uint16_t server_port, int size, LoggerBase &logger ) :
		client_port( client_port ),
		server_port( server_port ),
		server_ip( server_ip ),
		stop( false ),
		new_connection( false ),
		listener( true ),
		pool( size ),
		worker_on_hold( false ),
		logger( logger )
	{}

	// Takes incoming connection socket, creates a session and puts it into working list
	void handle_request( TcpSocket &&s )
	{
		// Raise the flag and wait until worker lets us to proceed
		new_connection = true;
		std::unique_lock lk( session_mtx );
		cv.wait( lk, [this]{return worker_on_hold;} );

		// Put new session into working pool
		sessions.push_back( std::make_shared<Session>( std::move( s ), server_ip, server_port, logger ) );
		log_debug( "Session pool size: %lu", sessions.size() );

		// Let worker to proceed
		new_connection = false;
		lk.unlock();
		cv.notify_one();
	}

	// Spawn worker thread
	void start_worker()
	{
		worker_thread = std::thread( &Private::worker, this );
	}

	// Worker loop, which tracks sessions in round robin order
	void worker()
	{
		while( !stop )
		{
			if ( new_connection )
			{
				// Let to accept incoming connection
				worker_on_hold = true;
				cv.notify_one();
				// Wait until done
				std::unique_lock lk( session_mtx );
				cv.wait(lk, [this]{return !new_connection;});
				worker_on_hold = false;
			}
			{
				std::lock_guard lk( session_mtx );
				unsigned processed_sessions = 0;
				for( auto it = sessions.begin(); it != sessions.end(); )
				{
					auto s = *it;
					if ( s->processing() )
					{
						it++;
						continue; // Skip
					}
					if ( !s->operator bool() )
					{
						it = sessions.erase( it );
						if ( it == sessions.end() )
						{
							break;
						}
						continue;
					}
					if ( s->work_pending() )
					{
						s->start_processing( pool.get() );
						processed_sessions++;
					}
					it++;
				}
				if ( processed_sessions == 0 )
				{
					// Lazy loop (take a sleep if nothing to process)
					std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
				}
			}
		}

	}
	void finalize()
	{
		pool.join();
		worker_thread.join();
	}
};

Proxy::Proxy( uint16_t client_port,
		const std::string &server_ip, uint16_t server_port,
		LoggerBase &logger, int threads ) :
	data_( new Private( client_port, server_ip, server_port, threads, logger ) )
{}

Proxy::~Proxy()
{}

bool Proxy::run()
{
	if ( !data_->listener.bind( data_->client_port ) )
	{
		log_error( "Failed to bind" );
		return false;
	}
	if ( !data_->listener.listen() )
	{
		log_error( "Failed to listen" );
		return false;
	}
	data_->start_worker();
	log_debug( "*** Proxy started ***" );
	while( !data_->stop )
	{
		if ( !data_->listener.wait( true, true, 1 ) )
		{
			continue;
		}
		auto client = data_->listener.accept();
		if ( !client )
		{
			continue;
		}
		data_->handle_request( std::move( client ) );
	}
	data_->finalize();
	return true;
}

void Proxy::stop()
{
	log_debug( "*** Stopping proxy ***" );
	data_->stop = true;
}
