#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "epoll.hpp"

using namespace std::chrono_literals;

struct Pipe
{
	int pipe_fd[2];
	int& pipe_r_fd;
	int& pipe_w_fd;
	Pipe() :
		pipe_r_fd(pipe_fd[0]),
		pipe_w_fd(pipe_fd[1])
	{
		assert(pipe2(pipe_fd, O_NONBLOCK) == 0);
	}
	~Pipe()
	{
		close(pipe_r_fd);
		close(pipe_w_fd);
	}
};

void wait_timeout()
{
	Pipe p;
	std::atomic<int> ctr(0);
	Epoll poll([&](int /*fd*/, Epoll::Event /*e*/) -> void{
		assert(false);
	});
	assert(poll.Add(p.pipe_r_fd, Epoll::Events({Epoll::Event::In})));
	auto start = std::chrono::steady_clock::now();
	assert(poll.Wait(100ms));
	assert(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() >= 100);
	assert(ctr.load() == 0);
}

void read_write()
{
	Pipe p;
	std::atomic<int> r_ctr(0);
	std::atomic<int> w_ctr(0);
	Epoll poll([&](int fd, Epoll::Event e) -> void{
		assert(false);
	});
	assert(poll.Add(p.pipe_r_fd, Epoll::Events({Epoll::Event::In}),
		[&](int fd, Epoll::Event e) -> void{
			assert(fd == p.pipe_r_fd);
			assert(e == Epoll::Event::In);
			r_ctr++;
		}));
	assert(poll.Add(p.pipe_w_fd, Epoll::Events({Epoll::Event::Out}),
		[&](int fd, Epoll::Event e) -> void{
			assert(fd == p.pipe_w_fd);
			assert(e == Epoll::Event::Out);
			if (w_ctr.load() == 0)
			{
				w_ctr++;
				char buf[] = {'A'};
				write(p.pipe_w_fd, buf, 1);
			}
		}));
	assert(poll.Wait(50ms));
	assert(poll.Wait(50ms));
	assert(r_ctr.load() == 1);
	assert(w_ctr.load() == 1);
}

void default_handler()
{
	Pipe p;
	std::atomic<int> ctr(0);
	Epoll poll([&](int fd, Epoll::Event e) -> void{
		assert(fd == p.pipe_r_fd);
		assert(e == Epoll::Event::In);
		ctr++;
		char buf[5];
		assert(read(p.pipe_r_fd, buf, sizeof(buf)) == 1);
		assert(buf[0] == 'A');
	});
	assert(poll.Add(p.pipe_r_fd, Epoll::Events({Epoll::Event::In})));
	{
		char buf[] = {'A'};
		write(p.pipe_w_fd, buf, 1);
	}
	assert(poll.Wait(50ms));
	assert(ctr.load() == 1);
}

void remove()
{
	Pipe p;
	std::atomic<int> ctr(0);
	Epoll poll([&](int fd, Epoll::Event e) -> void{
		assert(false);
	});
	assert(poll.Add(p.pipe_r_fd, Epoll::Events({Epoll::Event::In}),
		[&](int fd, Epoll::Event e) -> void{
			assert(fd == p.pipe_r_fd);
			assert(e == Epoll::Event::In);
			ctr++;
			char buf[5];
			assert(read(p.pipe_r_fd, buf, sizeof(buf)) == 1);
			assert(buf[0] == 'A');
		}));
	char buf[] = {'A'};
	write(p.pipe_w_fd, buf, 1);
	assert(poll.Wait(50ms));
	write(p.pipe_w_fd, buf, 1);
	assert(poll.Wait(50ms));
	write(p.pipe_w_fd, buf, 1);
	poll.Remove(p.pipe_r_fd);
	assert(poll.Wait(50ms));
	assert(ctr.load() == 2);
}

void stop_wait()
{
	Pipe p;
	std::atomic<int> ctr(0);
	Epoll poll([&](int fd, Epoll::Event e) -> void{
		assert(false);
	});
	assert(poll.Add(p.pipe_r_fd, Epoll::Events({Epoll::Event::In}),
		[&](int fd, Epoll::Event e) -> void{
			assert(fd == p.pipe_r_fd);
			assert(e == Epoll::Event::In);
			ctr++;
		}));
	std::thread t([&](){
		std::this_thread::sleep_for(10ms);
		poll.StopWait();
	});
	auto start = std::chrono::steady_clock::now();
	assert(poll.Wait());
	assert(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < 20);
	assert(ctr.load() == 0);
	t.join();
}

void one_shot()
{
	Pipe p;
	std::atomic<int> ctr(0);
	Epoll poll([&](int fd, Epoll::Event e) -> void{
		assert(false);
	});
	assert(poll.Add(p.pipe_r_fd, Epoll::Events({Epoll::Event::In, Epoll::Event::OneShot}),
		[&](int fd, Epoll::Event e) -> void{
			assert(fd == p.pipe_r_fd);
			assert(e == Epoll::Event::In);
			ctr++;
			char buf[5];
			assert(read(p.pipe_r_fd, buf, sizeof(buf)) == 1);
			assert(buf[0] == 'A');
		}));
	char buf[] = {'A'};
	// Toggle event
	write(p.pipe_w_fd, buf, 1);
	assert(poll.Wait(50ms));
	assert(ctr.load() == 1);
	// Check if triggering
	write(p.pipe_w_fd, buf, 1);
	assert(poll.Wait(50ms));
	assert(ctr.load() == 1);
	// Re-arm
	assert(poll.Modify(p.pipe_r_fd, {Epoll::Event::In}));
	assert(poll.Wait(50ms));
	assert(ctr.load() == 2);
}

void edge_trigger()
{
	Pipe p;
	std::atomic<int> ctr(0);
	Epoll poll([&](int fd, Epoll::Event e) -> void{
		assert(false);
	});
	assert(poll.Add(p.pipe_r_fd, Epoll::Events({Epoll::Event::In, Epoll::Event::EdgeTrigger}),
		[&](int fd, Epoll::Event e) -> void{
			assert(fd == p.pipe_r_fd);
			assert(e == Epoll::Event::In);
			ctr++;
			char buf[5];
			assert(read(p.pipe_r_fd, buf, 1) == 1);
			assert(buf[0] == 'A');
		}));
	assert(poll.Wait(50ms));
	assert(ctr.load() == 0);
	char buf[] = {'A'};
	// Toggle event
	write(p.pipe_w_fd, buf, 1);
	write(p.pipe_w_fd, buf, 1);
	for(int i = 0; i < 3; i++)
	{
		assert(poll.Wait(50ms));
	}
	assert(ctr.load() == 1);
	// Write more
	write(p.pipe_w_fd, buf, 1);
	assert(poll.Wait(50ms));
	assert(ctr.load() == 2);
}

int main()
{
	wait_timeout();
	read_write();
	default_handler();
	remove();
	stop_wait();
	one_shot();
	edge_trigger();
	return 0;
}