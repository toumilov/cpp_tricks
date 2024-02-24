#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <stdio.h>
#include <map>
#include "epoll.hpp"

#define COUNTOF(X) (sizeof(X) / sizeof(X[0]))

struct Epoll::Impl
{
	Impl(Callback common_cb) :
		m_fd(epoll_create(1)),
		m_wake_fd(eventfd(0, EFD_NONBLOCK)),
		m_cb(common_cb),
		m_handlers()
	{
		struct epoll_event evt{0};
		evt.data.fd = m_wake_fd;
		evt.events = EPOLLIN;
		if (m_wake_fd >= 0 && epoll_ctl(m_fd, EPOLL_CTL_ADD, m_wake_fd, &evt) < 0)
		{
			close(m_wake_fd);
			m_wake_fd = 0;
		}
	}
	Impl(Impl&) = delete;
	Impl(Impl&&) = delete;
	Impl& operator=(Impl&) = delete;
	Impl& operator=(Impl&&) = delete;
	~Impl()
	{
		if (m_wake_fd > 0)
		{
			close(m_wake_fd);
		}
		if (m_fd > 0)
		{
			close(m_fd);
		}
	}
	operator bool() const
	{
		return m_fd > 0;
	}

	bool Add(int fd, uint32_t e, Callback cb)
	{
		if (!operator bool() || e == 0)
		{
			return false;
		}
		struct epoll_event event{0};
		event.events = e;
		event.data.fd = fd;
		bool ret = epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &event) == 0;
		if (ret && cb)
		{
			m_handlers.emplace(fd, cb);
		}
		return ret;
	}

	bool Remove(int fd)
	{
		if (!operator bool())
		{
			return false;
		}
		auto it = m_handlers.find(fd);
		if (it != m_handlers.end())
		{
			m_handlers.erase(it);
		}
		return epoll_ctl(m_fd, EPOLL_CTL_DEL, fd, nullptr) == 0;
	}

	bool Modify(int fd, uint32_t e)
	{
		if (!operator bool())
		{
			return false;
		}
		struct epoll_event event{0};
		event.events = e;
		event.data.fd = fd;
		return epoll_ctl(m_fd, EPOLL_CTL_MOD, fd, &event) == 0;
	}

	bool Wait(const timespec* timeout) const
	{
		if (!operator bool())
		{
			return false;
		}
		struct epoll_event events[64];
		int n = epoll_pwait2(m_fd, events, COUNTOF(events), timeout, nullptr);
		for(int i = 0; i < n; i++)
		{
			if (events[i].data.fd == m_wake_fd)
			{
				// Wakeup requested
				eventfd_t val;
				eventfd_read(m_wake_fd, &val);
				break;
			}
			if (events[i].events & (EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP))
			{
				auto& cb = GetCallback(events[i].data.fd);
				if (cb)
				{
					cb(events[i].data.fd, Event::Hangup);
				}
			}
			else if (events[i].events & EPOLLIN)
			{
				auto& cb = GetCallback(events[i].data.fd);
				if (cb)
				{
					cb(events[i].data.fd, Event::In);
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				auto& cb = GetCallback(events[i].data.fd);
				if (cb)
				{
					cb(events[i].data.fd, Event::Out);
				}
			}
		}
		return n >= 0;
	}

	void StopWait() const
	{
		if (!operator bool())
		{
			return;
		}
		eventfd_write(m_wake_fd, 1);
	}

private:
	int m_fd;
	int m_wake_fd;
	mutable Callback m_cb;
	mutable std::map<int, Callback> m_handlers;

	Callback& GetCallback(int fd) const
	{
		auto it = m_handlers.find(fd);
		if (it != m_handlers.end())
		{
			return it->second;
		}
		return m_cb;
	}
};

Epoll::Events::Events(std::initializer_list<Event> l) :
	e(0)
{
	for(const auto i : l)
	{
		switch(i)
		{
		case Event::In:
			e |= EPOLLIN;
			break;
		case Event::Out:
			e |= EPOLLOUT;
			break;
		case Event::Hangup:
			e |= EPOLLRDHUP | EPOLLERR | EPOLLHUP;
			break;
		case Event::OneShot:
			e |= EPOLLONESHOT;
			break;
		case Event::EdgeTrigger:
			e |= EPOLLET;
			break;
		}
	}
}

Epoll::Epoll() :
	m_impl(new Impl(nullptr))
{}

Epoll::Epoll(Callback cb) :
	m_impl(new Impl(cb))
{}

Epoll::Epoll(Epoll&& rhs) :
	m_impl(std::move(rhs.m_impl))
{}

Epoll& Epoll::operator=(Epoll&& rhs)
{
	m_impl.swap(rhs.m_impl);
	return *this;
}

Epoll::~Epoll()
{}

Epoll::operator bool() const
{
	return m_impl->operator bool();
}

bool Epoll::Add(int fd, const Events& evt, Callback cb)
{
	if (fd < 0)
	{
		return false;
	}
	return m_impl->Add(fd, evt.e, cb);
}

bool Epoll::Remove(int fd)
{
	return m_impl->Remove(fd);
}

bool Epoll::Modify(int fd, const Events& evt)
{
	return m_impl->Modify(fd, evt.e);
}

bool Epoll::Wait(const timespec* duration) const
{
	return m_impl->Wait(duration);
}

bool Epoll::Wait() const
{
	return m_impl->Wait(nullptr);
}

void Epoll::StopWait() const
{
	return m_impl->StopWait();
}