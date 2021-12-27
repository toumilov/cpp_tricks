
#include <string>
#include <stdio.h>

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Argument expected\n");
		return 1;
	}
	std::string cmd;
	for (int i = 1; i < argc; i++)
	{
		if (i > 1)
		{
			cmd.append(" ");
		}
		cmd.append(argv[i]);
	}
	cmd.append("\n");
	FILE *fd = popen(cmd.c_str(), "r");
	if (fd)
	{
		std::string out;
		char buf[100];
		while(!ferror(fd) && !feof(fd))
		{
			size_t n = fread(buf, 1, sizeof(buf), fd);
			if (n > 0)
			{
				out.append(buf, n);
			}
		}
		pclose(fd);
		printf("<%s>\n", out.c_str());
	} else {
		perror("Failed to run command: ");
	}
	return 0;
}
