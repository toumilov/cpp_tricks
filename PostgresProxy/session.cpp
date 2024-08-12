#include <algorithm>
#include <functional>
#include <memory>
#include <arpa/inet.h>
#include "session.hpp"

enum PostresqlQueryTypes
{
	SimpleQuery = 'Q'
};

Session::Session( TcpSocket &&client, const std::string &server_ip, uint16_t server_port, LoggerBase &logger ) :
	client_( std::move( client ) ),
	processing_( false ),
	requests_processed_( 0 ),
	logger_( logger )
{
	// Connect to server
	server_.connect( server_ip.c_str(), server_port );
	if ( server_ )
	{
		log_debug( "Client '%s' session started", get_id().c_str() );
	} else {
		log_debug( "Client '%s' session start rejected by server", get_id().c_str() );
	}
}

Session::Session( Session &&rhs ) :
	client_( std::move( rhs.client_ ) ),
	server_( std::move( rhs.server_ ) ),
	processing_( rhs.processing_.load() ),
	requests_processed_( rhs.requests_processed_ ),
	logger_( rhs.logger_ )
{}

Session::~Session()
{
	log_debug( "Client '%s' session ended", get_id().c_str() );
}

Session& Session::operator=( Session &&rhs )
{
	client_.swap( rhs.client_ );
	server_.swap( rhs.server_ );
	processing_.store( rhs.processing_ );
	std::swap( requests_processed_, rhs.requests_processed_ );
	req_buf.swap( rhs.req_buf );
	return *this;
}

Session::operator bool() const
{
	return client_ && server_;
}

bool Session::work_pending() const
{
	return client_.wait( true, false, 0 ) ||
		   server_.wait( true, false, 0 );
}

void Session::start_processing( ThreadPool::Worker &worker )
{
	processing_ = true;
	worker.run( std::bind( &Session::process, this ) );
}

bool Session::process()
{
	std::unique_ptr<int,std::function<void(int*)>> guard( new int, [this](int *p){
		processing_ = false; // Scoped rollback trick
		delete( p );
	});
	bool ret = handle_client_request() && handle_server_response();
	if ( !ret )
	{
		client_.close();
		server_.close();
	}
	return ret;
}

bool Session::processing() const
{
	return processing_.load();
}

std::string Session::get_id() const
{
	return client_.peer_ip() + ":" + std::to_string( client_.peer_port() );
}

bool Session::handle_client_request()
{
	if ( !client_.wait( true, false, 0 ) )
	{
		return true; // Nothing to receive yet
	}
	// Read client request
	// 1. read type
	uint8_t req_type = '0';
	size_t bytes;
	if ( requests_processed_ > 0 )
	{
		// Start message doesn't have type tag
		client_.receive( (char*)&req_type, 1, bytes );
		if ( bytes == 0 ) { return false; }
	}
	requests_processed_++;
	// 2. read size
	uint32_t req_size_nbo;
	client_.receive( (char*)&req_size_nbo, sizeof( req_size_nbo ), bytes );
	if ( bytes < sizeof( req_size_nbo ) ) { return false; }
	uint32_t req_size = ntohl( req_size_nbo ); // Fix network byte sequence to LittleEndian
	req_size -= sizeof( req_size ); // Only payload size matters
	// 3. read payload
	if ( req_buf.size() < req_size )
	{
		req_buf.resize( req_size );
	}
	uint32_t bytes_read = 0;
	while( bytes_read < req_size )
	{
		if ( !client_.receive( req_buf.data() + bytes_read, req_size - bytes_read, bytes ) ||
			 bytes == 0 )
		{
			return false;
		}
		bytes_read += bytes;
	}

	// Filter request
	if ( req_type == SimpleQuery )
	{
		logger_.log( std::string( req_buf.data(), req_size ) );
	}

	// Forward to the server
	bool req_type_sent = true;
	if ( requests_processed_ > 1 )
	{
		// Start message doesn't have type tag
		req_type_sent = server_.send( (const char*)&req_type, sizeof( req_type ) );
	}
	return !( !req_type_sent ||
			  !server_.send( (const char*)&req_size_nbo, sizeof( req_size_nbo ) ) ||
			  !server_.send( (const char*)req_buf.data(), req_size ) );
}

bool Session::handle_server_response()
{
	if ( !server_.wait( true, false, 0 ) )
	{
		return true; // Nothing to receive yet
	}
	// Read server response
	// 1. read type
	uint8_t resp_type;
	size_t bytes;
	server_.receive( (char*)&resp_type, 1, bytes );
	if ( bytes == 0 ) { return false; }
	// 2. read size
	uint32_t resp_size_nbo;
	server_.receive( (char*)&resp_size_nbo, sizeof( resp_size_nbo ), bytes );
	if ( bytes < sizeof( resp_size_nbo ) ) { return false; }
	uint32_t resp_size = ntohl( resp_size_nbo ); // Fix network byte sequence to LittleEndian
	resp_size -= sizeof( resp_size ); // Only payload size matters
	// 3. read payload
	if ( req_buf.size() < resp_size )
	{
		req_buf.resize( resp_size );
	}
	uint32_t bytes_read = 0;
	while( bytes_read < resp_size )
	{
		if ( !server_.receive( req_buf.data() + bytes_read, resp_size - bytes_read, bytes ) ||
			 bytes == 0 )
		{
			return false;
		}
		bytes_read += bytes;
	}

	// Forward to the client
	return !( !client_.send( (const char*)&resp_type, sizeof( resp_type ) ) ||
			  !client_.send( (const char*)&resp_size_nbo, sizeof( resp_size_nbo ) ) ||
			  !client_.send( (const char*)req_buf.data(), resp_size ) );
}
