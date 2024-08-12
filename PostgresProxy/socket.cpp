#include <unistd.h>
#include <cstdio>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "logger.hpp"
#include "socket.hpp"

TcpSocket::TcpSocket( bool reuse_addr ) :
	fd_( 0 ),
	port_( 0 )
{
	fd_ = socket( AF_INET, SOCK_STREAM, 0 );
	if ( fd_ && reuse_addr )
	{
		int enable = 1;
		setsockopt( fd_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int) );
	}
}

TcpSocket::TcpSocket( int fd, const std::string &ip, uint16_t port ) :
	fd_( fd ),
	port_( port ),
	ip_( ip )
{}

TcpSocket::TcpSocket( TcpSocket &&rhs ) :
	fd_( rhs.fd_ ),
	port_( rhs.port_ ),
	ip_( rhs.ip_ )
{
	rhs.fd_ = 0;
	rhs.port_ = 0;
	rhs.ip_.clear();
}

TcpSocket::~TcpSocket()
{
	close();
}

TcpSocket& TcpSocket::operator=( TcpSocket &&rhs )
{
	if ( fd_ )
	{
		close();
	}
	std::swap( fd_, rhs.fd_ );
	std::swap( port_, rhs.port_ );
	std::swap( ip_, rhs.ip_ );
	return *this;
}

TcpSocket::operator bool() const
{
	return fd_ > 0;
}

void TcpSocket::swap( TcpSocket &rhs )
{
	std::swap( fd_, rhs.fd_ );
}

void TcpSocket::close()
{
	if ( fd_ > 0 )
	{
		::close( fd_ );
		fd_ = 0;
	}
}

bool TcpSocket::bind( uint16_t port ) const
{
	if ( !operator bool() )
	{
		return false;
	}
	struct sockaddr_in sock_addr;
	std::memset( &sock_addr, 0, sizeof(sock_addr) );
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons( port );
	if ( ::bind( fd_, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) < 0 )
	{
		log_error( "Bind failed" );
		return false;
	}
	return true;
}

bool TcpSocket::listen( int backlog ) const
{
	if ( !operator bool() )
	{
		return false;
	}
	if ( ::listen( fd_, backlog ) )
	{
		log_error( "Listen failed" );
		return false;
	}
	return true;
}

TcpSocket TcpSocket::accept() const
{
	if ( !operator bool() )
	{
		return TcpSocket();
	}
	struct sockaddr_in addr;
	socklen_t len = sizeof( addr );
	std::memset( &addr, 0, len );
	int fd = ::accept( fd_, (sockaddr*)&addr, &len );
	if ( fd < 0 )
	{
		return TcpSocket();
	}
	char buf[16] = {0};
	if ( addr.sin_family == AF_INET )
	{
		inet_ntop( AF_INET, &addr.sin_addr, buf, sizeof( buf ) );
	}
	return TcpSocket( fd, std::string( buf ), ntohs( addr.sin_port ) );
}

bool TcpSocket::connect( const char *ip, uint16_t port )
{
	if ( !operator bool() )
	{
		return false;
	}
	struct sockaddr_in addr;
	memset( &addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons( port );
	inet_pton( AF_INET, ip, &(addr.sin_addr) );
	if ( ::connect( fd_, (struct sockaddr*)&addr, sizeof( struct sockaddr_in ) ) < 0 )
	{
		return false;
	}
	return true;
}

bool TcpSocket::receive( char *buf, size_t size, size_t &bytes ) const
{
	if ( !operator bool() )
	{
		return false;
	}
	ssize_t r = ::recv( fd_, (void*)buf, size, 0 );
	if ( r < 0 )
	{
		return false;
	} else {
		bytes = r;
	}
	return true;
}

bool TcpSocket::send( const char *buf, size_t size ) const
{
	if ( !operator bool() )
	{
		return false;
	}
	bool ret = true;
	do
	{
		ssize_t s = ::send( fd_, buf, size, 0 );
		if ( s < 0 )
		{
			ret = false;
			break;
		}
		size -= (size_t)s;
		buf += s;
	} while( size > 0 );
	return ret;
}

bool TcpSocket::wait( bool read, bool write, unsigned sec ) const
{
	if ( !operator bool() )
	{
		return false;
	}
	fd_set fds;
	struct timeval tv;

	FD_ZERO( &fds );
	FD_SET( fd_, &fds );
	tv.tv_sec = sec;
	tv.tv_usec = 0;
	if ( ::select( fd_ + 1, read ? &fds : NULL, write ? &fds : NULL, NULL, &tv ) <= 0 )
	{
		return false;
	}
	return true;
}

uint16_t TcpSocket::peer_port() const
{
	return port_;
}

std::string TcpSocket::peer_ip() const
{
	return ip_;
}
