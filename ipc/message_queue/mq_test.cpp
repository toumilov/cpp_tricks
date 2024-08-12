#include <cstring>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <mqueue.h>
#include <thread>
#include <chrono>


int main()
{
	const char *mq_name = "/at_test";
	switch(fork())
	{
		case -1:
			perror("fork");
			break;
		case 0: // Client
		{
			printf("=== Client (%d) ===\n", getpid());
			mqd_t mq_fd = mq_open(mq_name, O_RDWR);
			if (mq_fd < 0)
			{
				perror("open");
				return 1;
			}
			char buf[10];
			unsigned prio;
			struct timespec ts;
			ts.tv_nsec = 0;
			for(int i = 0; i < 5; i++)
			{
				ts.tv_sec = time(nullptr) + 1;
				auto bytes = mq_timedreceive(mq_fd, buf, sizeof(buf), &prio, &ts);
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (bytes < 0)
				{
					perror("recv");
					continue;
				}
				printf("[%s]\n", buf);
			}
			std::this_thread::sleep_for(std::chrono::seconds(3));
			printf("==== Client END ====\n");
			break;
		}
		default:
		{
			printf("=== Server ===\n");
			mq_attr attr = {0, 10, 10, 0};
			mqd_t mq_fd = mq_open(mq_name, O_CREAT | O_WRONLY, 0666, &attr);
			if (mq_fd < 0)
			{
				perror("open");
				return 1;
			}
			char buf[10];
			struct timespec ts;
			ts.tv_nsec = 0;
			for(int i = 0; i < 10; i++)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				snprintf(buf, sizeof(buf), "MSG(%d)", i);
				ts.tv_sec = time(nullptr) + 1;
				if (mq_timedsend(mq_fd, buf, 10, 0, &ts))
				{
					perror("send");
				}
			}
			mq_close(mq_fd);
			mq_unlink(mq_name);
			printf("==== END ====\n");
			break;
		}
	}

	return 0;
}