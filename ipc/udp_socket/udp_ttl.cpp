#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include <chrono>


class UDP
{
	int fd_;
	uint16_t port_;

public:
	UDP( uint16_t port, bool blocking = true ) :
		port_( port )
	{
		fd_ = socket( AF_INET, SOCK_DGRAM | ( blocking ? 0 : SOCK_NONBLOCK ), 0 );
		if ( fd_ < 0 )
		{
			perror( "socket" );
			exit(1);
		}
		int enable = 1;
		if ( setsockopt( fd_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof( enable ) ) < 0 /*||
			 setsockopt( fd_, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof( enable ) ) < 0*/ )
		{
			perror( "setsockopt" );
			exit(1);
		}
	}
	~UDP()
	{
		if ( fd_ )
		{
			close( fd_ );
		}
	}

	bool bind()
	{
		struct sockaddr_in sa;
		memset( &sa, 0, sizeof( sa ) );
		sa.sin_family = AF_INET;
		sa.sin_port = htons( port_ );
		sa.sin_addr.s_addr = htonl( INADDR_ANY );
		if ( ::bind( fd_, (struct sockaddr*)&sa, sizeof( sa ) ) < 0 )
		{
			perror( "bind" );
			return false;
		}
		return true;
	}

	bool send( const std::string &ip, void *ptr, size_t size )
	{
		struct sockaddr_in sa;
		memset( &sa, 0, sizeof( sa ) );
		sa.sin_family = AF_INET;
		sa.sin_port = htons( port_ );
		if ( !inet_pton( AF_INET, ip.c_str(), &( sa.sin_addr ) ) )
		{
			return false;
		}
		auto n = sendto( fd_, (void*)ptr, size, MSG_NOSIGNAL, (struct sockaddr*)&sa, sizeof( sa ) );
		if ( n < 0 )
		{
			perror( "sendto" );
			exit(1);
		}
		return size_t(n) == size;
	}

	bool receive( std::string *peer_ip, void *ptr, size_t size )
	{
		struct sockaddr_in peer_addr;
		socklen_t peer_len = sizeof( peer_addr );
		auto n = recvfrom( fd_, (void*)ptr, size, MSG_PEEK, (sockaddr*) &peer_addr, &peer_len );
		if ( n < 0 )
		{
			if ( errno == EAGAIN )
			{
				return false;
			}
			perror( "recvfrom" );
			exit(1);
		}
		if ( size_t(n) < size )
		{
			printf( "Message incomplete [%ld:%lu]\n", n, size );
			return false;
		}
		recvfrom( fd_, (void*)ptr, size, 0, nullptr, nullptr );
		if ( peer_ip )
		{
			char buf[16];
			inet_ntop( AF_INET, &peer_addr.sin_addr, buf, sizeof( buf ) );
			*peer_ip = std::string( buf );
		}
		return true;
	}
};

int main( int argc, char **argv)
{
	uint16_t port = 4555;
	switch(fork())
	{
		case -1:
		{
			perror("fork");
			return 1;
		}
		case 0:
		{
			printf( "=== Client (%d) ===\n", getpid() );
			UDP link( port );
			std::string data( "test" );
			if ( !link.send( "127.0.0.1", (void*)data.c_str(), data.size() + 1 ) )
			{
				printf( "Failed to send\n" );
				return 1;
			}
			printf( "Sent\n" );
			break;
		}
		default:
		{
			printf( "=== Server ===\n" );
			UDP link( port );
			link.bind();
			std::string ip;
			char buf[5] = {0};
			if ( !link.receive( &ip, buf, sizeof( buf ) ) )
			{
				printf( "Failed to receive\n" );
				return 1;
			}
			printf( "Received: '%s' from %s\n", buf, ip.c_str() );
			break;
		}
	}
	return 0;
}