
#include "local_socket.hpp"


int main()
{
	LocalSocket sock("/tmp/test.socket");
	if (!sock.bind())
	{
		perror("bind");
		return 1;
	}
	if (!sock.listen())
	{
		perror("listen");
		return 1;
	}
	while(true)
	{
		LocalSocket c = sock.accept();
		if (!c)
		{
			perror("accept");
			return 1;
		}
		int p, u, g;
		c.peer_credentials(&p, &u, &g);
		printf("Client: PID <%d> UID <%d> GID <%d>\n", p, u, g);
		std::string msg;
		msg.resize(10);
		size_t bytes;
		c.wait(true, false, 3);
		if (!c.receive((char*)msg.data(), msg.size(), bytes))
		{
			perror("read");
			continue;
		}
		printf("Received %lu bytes: [%s]\n", bytes, msg.c_str());
		msg.resize(bytes);
		msg += "++";
		if (!c.send(msg.data(), msg.size()))
		{
			perror("write");
			continue;
		}
		printf("Sent: [%s]\n", msg.c_str());
	}
	return 0;
}
