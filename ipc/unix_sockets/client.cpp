
#include "local_socket.hpp"


int main()
{
	LocalSocket sock;
	if (!sock.connect("/tmp/test.socket"))
	{
		perror("connect");
		return 1;
	}
	std::string msg("Test");
	if (!sock.send(msg.data(), msg.size()))
	{
		perror("write");
		return 1;
	}
	msg.resize(10);
	size_t bytes;
	if (!sock.wait(true, false, 3))
	{
		perror("wait");
		return 1;
	}
	if (!sock.receive(&msg[0], msg.size(), bytes))
	{
		perror("read");
		return 1;
	}
	printf("Received %lu bytes: [%s]\n", bytes, msg.c_str());
	return 0;
}
