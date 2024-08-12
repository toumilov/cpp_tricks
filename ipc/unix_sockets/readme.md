# Unix socket client/server

Unix (local) sockets are the standard transport means for daemon services. Along with their speed (comparing to network sockets), they provide a way for client authentication (peer credentials).

## Local socket interface
Below is the sample socket C++ wrapper for more convenient use.
```
class LocalSocket
{
public:
	LocalSocket();
	LocalSocket(const std::string &path, int fd = -1);
	~LocalSocket();

	inline void close() { if (fd_ > 0) ::close(fd_); }
	inline operator bool() const { return fd_ > 0; }
	bool bind() const;
	bool listen(int backlog = 5) const;
	LocalSocket accept() const;
	bool connect(const std::string &path);
	bool receive(char *buf, size_t size, size_t &bytes) const;
	bool send(const char *buf, size_t size) const;
	bool wait(bool read, bool write, unsigned sec) const;
	bool peer_credentials(int *pid, int *uid, int *gid) const;

private:
	int fd_;
	std::string path_;
};
```

## Client
Normal client flow is:
* Create socket
* Connect
* Send
* (**optional**) Wait for response
* Receive

**Sample:** *client.cpp*

## Server
Normal server flow is:
* Create socket
* Bind to address
* Listen for incoming connections
	* Accept client connection
	* (**optional**) Check peer credentials
	* Wait for request
	* Process request
	* Send response

**Sample:** *server.cpp*
