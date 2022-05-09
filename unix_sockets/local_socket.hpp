
#include <unistd.h>
#include <string>


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
