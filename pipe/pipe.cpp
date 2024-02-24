#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <chrono>
#include <thread>


int main()
{
	int pipe_fd[2];
	if (pipe2(pipe_fd, O_NONBLOCK) < 0)
	{
		perror("pipe2");
		return 1;
	}
	int& pipe_r_fd = pipe_fd[0];
	int& pipe_w_fd = pipe_fd[1];

	std::thread t([](int fd){
		for(char c = 'A'; c <= 'F'; c++)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			write(fd, &c, 1);
		}
	}, pipe_w_fd);

	// Wait for data
	struct pollfd pfd {pipe_r_fd, POLLIN, 0};
	char buf[10] = {0};
	while(true)
	{
		printf(".\n");
		int rc = poll(&pfd, 1, 3000);
		if (rc < 0)
		{
			perror("poll");
			return 1;
		}
		if (rc == 0)
		{
			continue;
		}
		rc = read(pipe_r_fd, buf, sizeof(buf));
		switch(rc)
		{
		case -1:
			perror("read");
			return 1;
		case 0:
			break;
		default:
			printf("<%.*s>\n", rc, buf);
		}
		if (buf[0] == 'F')
		{
			break;
		}
	}
	t.join();
	close(pipe_r_fd);
	close(pipe_w_fd);
	return 0;
}