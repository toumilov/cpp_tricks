#pragma once
#include <atomic>
#include <vector>
#include "socket.hpp"
#include "logger.hpp"
#include "thread_pool.hpp"

class Session
{
public:
	Session( TcpSocket &&client, const std::string &server_ip, uint16_t server_port, LoggerBase &logger );
	Session( const Session& ) = delete;
	Session( Session&& );
	~Session();
	Session& operator=( const Session& ) = delete;
	Session& operator=( Session&& );
	operator bool() const;
	bool work_pending() const;
	void start_processing( ThreadPool::Worker &worker );
	bool processing() const;
	std::string get_id() const;

private:
	TcpSocket client_;
	TcpSocket server_;
	std::atomic_bool processing_;
	unsigned requests_processed_;
	std::vector<char> req_buf;
	LoggerBase &logger_;

	bool process();
	bool handle_client_request();
	bool handle_server_response();
};
