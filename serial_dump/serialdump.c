#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

volatile int g_stop;

void print_usage(char* app)
{
	printf("Serial port logger tool\n");
	printf("Usage:\n");
	printf("%s <device> [options]\n", app);
	printf("-t: print timestamp\n");
	printf("-b <baud rate>\n");
	printf("   9600\n");
	printf("   19200\n");
	printf("   38400\n");
	printf("   57600\n");
	printf("   115200 (default)\n");
	printf("   230400\n");
	printf("   460800\n");
	printf("   500000\n");
	printf("   576000\n");
	printf("   921600\n");
	printf("   1000000\n");
	printf("   1152000\n");
	printf("   1500000\n");
	printf("   2000000\n");
	printf("   2500000\n");
	printf("   3000000\n");
	printf("   3500000\n");
	printf("   4000000\n");
	printf("-5: word len = 5\n");
	printf("-6: word len = 6\n");
	printf("-7: word len = 7\n");
	printf("-8: word len = 8 (default)\n");
	printf("-s <stop bits>\n");
	printf("   1 (default)\n");
	printf("   2\n");
	printf("-e: parity = even\n");
	printf("-o: parity = odd\n");
	printf("-n: parity = none (default)\n");
}

int parse_args(int argc, char** argv, struct termios* term, int* print_time)
{
	int i, num;
	speed_t speed;
	char* end;

	if (argc < 3)
	{
		return 1; // Missing arguments
	}

	*print_time = 0;
	memset(term, 0, sizeof(struct termios));

	// Set defaults
	speed = B115200;

	// CREAD  - enable receiver
	// CLOCAL - ignore modem control lines
	term->c_cflag = term->c_cflag | (CREAD | CLOCAL);
	term->c_cflag = term->c_cflag | CS8; // Word len = 8

	for(i = 2; i < argc; i++)
	{
		if (!strcmp(argv[i], "-t"))
		{
			*print_time = 1;
			continue;
		}

		// Baud rate
		if (!strcmp(argv[i], "-b"))
		{
			i++;
			if (i >= argc)
			{
				return 1; // Missing argument
			}
			num = strtoul(argv[i], &end, 10);
			if (errno != 0)
			{
				return 1; // Bad argument
			}
			switch(num)
			{
				case 9600:
					speed = B9600;
					break;
				case 19200:
					speed = B19200;
					break;
				case 38400:
					speed = B38400;
					break;
				case 57600:
					speed = B57600;
					break;
				case 115200:
					speed = B115200;
					break;
				case 230400:
					speed = B230400;
					break;
				case 460800:
					speed = B460800;
					break;
				case 500000:
					speed = B500000;
					break;
				case 576000:
					speed = B576000;
					break;
				case 921600:
					speed = B921600;
					break;
				case 1000000:
					speed = B1000000;
					break;
				case 1152000:
					speed = B1152000;
					break;
				case 1500000:
					speed = B1500000;
					break;
				case 2000000:
					speed = B2000000;
					break;
				case 2500000:
					speed = B2500000;
					break;
				case 3000000:
					speed = B3000000;
					break;
				case 3500000:
					speed = B3500000;
					break;
				case 4000000:
					speed = B4000000;
					break;
				default:
					return 1; // Bad option
			}
			continue;
		}
		// Word length
		if (!strcmp(argv[i], "-5"))
		{
			term->c_cflag = term->c_cflag | CS5;
			continue;
		}
		if (!strcmp(argv[i], "-6"))
		{
			term->c_cflag = term->c_cflag | CS6;
			continue;
		}
		if (!strcmp(argv[i], "-7"))
		{
			term->c_cflag = term->c_cflag | CS7;
			continue;
		}
		if (!strcmp(argv[i], "-8"))
		{
			term->c_cflag = term->c_cflag | CS8;
			continue;
		}
		// Stop bits
		if (!strcmp(argv[i], "-s"))
		{
			i++;
			if (i >= argc)
			{
				return 1; // Missing argument
			}
			num = strtoul(argv[i], &end, 10);
			if (errno != 0)
			{
				return 1; // Bad argument
			}
			switch(num)
			{
				case 1:
					term->c_cflag &= ~CSTOPB;
					break;
				case 2:
					term->c_cflag |= CSTOPB;
					break;
				default:
					return 1; // Bad option
			}
			continue;
		}
		if (!strcmp(argv[i], "-o"))
		{
			term->c_iflag = term->c_iflag | INPCK;
			term->c_iflag = term->c_iflag | PARODD;
			term->c_iflag = term->c_iflag | PARENB;
			continue;
		}
		if (!strcmp(argv[i], "-e"))
		{
			term->c_iflag = term->c_iflag | INPCK;
			term->c_iflag = term->c_iflag | PARENB;
			continue;
		}
		if (!strcmp(argv[i], "-n"))
		{
			term->c_iflag = term->c_iflag & ~INPCK;
			term->c_iflag = term->c_iflag & ~PARODD;
			term->c_iflag = term->c_iflag & ~PARENB;
			continue;
		}
		return 1; // Unknown argument
	}


	// Set speed
	if ((cfsetospeed(term, speed) < 0) || (cfsetispeed(term, speed) < 0))
	{
		return 1; // Failed to set speed
	}
	return 0;
}

void term_handler(int)
{
	g_stop = 1;
}

int main(int argc, char** argv)
{
	int fd, ret, i, print_time;
	ssize_t bytes;
	struct termios term;
	fd_set fds;
	struct timeval tv;
	char buf[100];
	char time_buf[100];
	struct tm timestamp;

	if (parse_args(argc, argv, &term, &print_time))
	{
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

	// Open TTY
	fd = open(argv[1], O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY);
	if (fd < 0)
	{
		printf("Failed to open device\n");
		return EXIT_FAILURE;
	}

	do
	{
		// Apply settings
		if (tcsetattr(fd, TCSANOW, &term) < 0)
		{
			printf("Failed setup device\n");
			break;
		}

		// Flush buffers
		tcflush(fd, TCIOFLUSH);

		// Setup signal handler
		g_stop = 0;
		signal(SIGINT, term_handler);

		// Read data
		while(g_stop == 0)
		{
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			ret = select(fd + 1, &fds, NULL, NULL, &tv);
			if (ret == -1)
			{
				break;
			}
			else if (ret)
			{
				bytes = read(fd, buf, sizeof(buf));
				if (bytes < 1)
				{
					continue;
				}
				if (print_time)
				{
					gettimeofday(&tv, NULL);
					localtime_r(&tv.tv_sec, &timestamp);
					strftime(time_buf, sizeof(time_buf), "%T", &timestamp);
					printf("%s:%.3lu ", time_buf, tv.tv_usec / 1000);
				}
				for (i = 0; i < bytes; i++)
				{
					printf("%X", buf[i]);
				}
				if (print_time)
				{
					printf("\n");
				}
				fflush(0);
			}
			else
			{
				continue; // Timeout
			}
		}
	} while(0);

	close(fd);

	return EXIT_SUCCESS;
}