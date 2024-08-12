#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "proxy.hpp"

static Proxy *proxy_ref = nullptr;

void interrupt_signal_handler( [[maybe_unused]] int signal )
{
	// Stop proxy on Ctrl+C
	proxy_ref->stop();
}

void usage( const char *self )
{
	printf( "Usage:\n" );
	printf( "%s <client port> <server IP> <server port(default = 5432)>\n", self );
}

int main( int argc, char **argv )
{
	static const unsigned default_postgres_port = 5432;

	// Setup Ctrl+C handler for graceful shutdown
	signal( SIGINT, interrupt_signal_handler );

	// Argument parsing
	auto read_uint_arg = [&]( unsigned &num, int arg, bool optional = false ) -> bool {
		unsigned n = 0;
		if ( argc > arg )
		{
			auto l = strlen( argv[arg] );
			char *end = argv[arg] + l;
			n = std::strtoul( argv[arg], &end, 10 );
			if ( errno || end != argv[arg] + l )
			{
				perror( "Wrong argument" );
				n = 0;
			}
		}
		if ( n )
		{
			num = n;
		}
		return n || optional;
	};
	unsigned client_port, server_port = default_postgres_port;
	if ( argc < 3 ||
		 !read_uint_arg( client_port, 1 ) ||
		 !read_uint_arg( server_port, 3, true ) )
	{
		usage( argv[0] );
		return 1;
	}

	// Setup tracing
	Trace::instance().setup( Trace::Level::Error ); // Debug

	// Setup query logger
	FileLogger logger( "log.txt" );

	// Instantiate proxy
	Proxy proxy( client_port, argv[2], server_port, logger );
	proxy_ref = &proxy;
	proxy.run();

	return 0;
}