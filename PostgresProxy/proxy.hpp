#pragma once
#include <memory>
#include "logger.hpp"

class Proxy
{
public:
	/*
	 * @param[in] client_port - TCP port for client connections
	 * @param[in] server_ip - PostgreSQL server IP address
	 * @param[in] server_port - PostgreSQL server TCP port
	 * @param[in] logger - query logger object
	 * @param[in] threads - thread pool size
	 */
	Proxy( uint16_t client_port,
		const std::string &server_ip, uint16_t server_port,
		LoggerBase &logger, int threads = 5 );
	~Proxy();
	bool run();
	void stop();

private:
	struct Private;
	std::unique_ptr<Private> data_;
};
