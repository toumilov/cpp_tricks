#pragma once
#include <cstdint>
#include <cstring>
#include <string>

class TcpSocket
{
public:
	TcpSocket( bool reuse_addr = false );
	TcpSocket( const TcpSocket& ) = delete;
	TcpSocket( TcpSocket&& );
	~TcpSocket();

	TcpSocket& operator=( const TcpSocket& ) = delete;
	TcpSocket& operator=( TcpSocket&& );
	operator bool() const;
	void swap( TcpSocket& );
	void close();
	bool bind( uint16_t port ) const;
	bool listen( int backlog = 10 ) const;
	TcpSocket accept() const;
	bool connect( const char *ip, uint16_t port );
	bool receive( char *buf, size_t size, size_t &bytes ) const;
	bool send( const char *buf, size_t size ) const;
	bool wait( bool read, bool write, unsigned sec ) const;
	uint16_t peer_port() const;
	std::string peer_ip() const;

private:
	int fd_;
	uint16_t port_;
	std::string ip_;
	TcpSocket( int, const std::string&, uint16_t );
};
