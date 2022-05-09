
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include "local_socket.hpp"


LocalSocket::LocalSocket()
{
	fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
}

LocalSocket::LocalSocket(const std::string &path, int fd) :
	fd_(fd),
	path_(path)
{
	if (fd_ == -1 && (path_.length() <= (sizeof(sockaddr_un::sun_path) - 1)) &&
		(remove(path_.c_str()) == 0 || errno == ENOENT))
	{
		fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
	}
}

LocalSocket::~LocalSocket()
{
	close();
}

bool LocalSocket::bind() const
{
	if (!operator bool())
	{
		return false;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path_.c_str(), path_.length());
	return ::bind(fd_, (sockaddr*)&addr, sizeof(addr)) == 0;
}

bool LocalSocket::listen(int backlog) const
{
	if (!operator bool())
	{
		return false;
	}
	return ::listen(fd_, backlog) == 0;
}

LocalSocket LocalSocket::accept() const
{
	if (!operator bool())
	{
		return LocalSocket();
	}
	struct sockaddr_un addr;
	socklen_t len = sizeof(addr);
	memset(&addr, 0, len);
	int fd = ::accept(fd_, (sockaddr*)&addr, &len);
	return LocalSocket(std::string(addr.sun_path), fd);
}

bool LocalSocket::connect(const std::string &path)
{
	if (!operator bool() || (path.length() > (sizeof(sockaddr_un::sun_path) - 1)))
	{
		return false;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
	if (::connect(fd_, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1)
	{
		return false;
	}
	path_ = path;
	return true;
}

bool LocalSocket::receive(char *buf, size_t size, size_t &bytes) const
{
	if (!operator bool())
	{
		return false;
	}
	ssize_t r = ::recv(fd_, (void*)buf, size, 0);
	if (r < 0)
	{
		return false;
	}
	bytes = r;
	return true;
}

bool LocalSocket::send(const char *buf, size_t size) const
{
	if (!operator bool())
	{
		return false;
	}
	do
	{
		ssize_t s = ::send(fd_, buf, size, 0);
		if (s < 0)
		{
			return false;
		}
		size -= (size_t)s;
		buf += s;
	} while(size > 0);
	return true;
}

bool LocalSocket::wait(bool read, bool write, unsigned sec) const
{
	if (!operator bool())
	{
		return false;
	}
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(fd_, &fds);
	tv.tv_sec = sec;
	tv.tv_usec = 0;
	return select(fd_ + 1, read ? &fds : NULL, write ? &fds : NULL, NULL, &tv) > 0;
}

bool LocalSocket::peer_credentials(int *pid, int *uid, int *gid) const
{
	struct ucred cred;
	socklen_t len = sizeof(cred);
	if (getsockopt(fd_, SOL_SOCKET, SO_PEERCRED, &cred, &len) == 0 && len == sizeof(cred))
	{
		if (pid) *pid = cred.pid;
		if (uid) *uid = cred.uid;
		if (gid) *gid = cred.gid;
		return true;
	}
	return false;
}
