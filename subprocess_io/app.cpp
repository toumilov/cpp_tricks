
#include <unistd.h>
#include <string.h>
#include <string>
#include <map>
#include <utility>
#include <functional>


#define print( format, ... ) printf((format), ## __VA_ARGS__);fflush(stdout);


class TestApp
{
public:
	TestApp() :
		quit_(false),
		buf_("test\n")
	{
		commands_ = {
			{"quit", {"Exit application", std::bind(&TestApp::quit, this)}},
			{"in", {"Stdin test", std::bind(&TestApp::in, this)}},
			{"out", {"Stdout test", std::bind(&TestApp::out, this)}}
		};
	}

	operator bool() const
	{
		return quit_;
	}

	void usage() const
	{
		print("Usage:\n");
		for(auto &i : commands_)
		{
			print(" %s - %s\n", i.first.c_str(), i.second.first.c_str());
		}
	}

	void get_command()
	{
		print("Enter command: ");
		char cmd[10];
		if (fgets(cmd, sizeof(cmd), stdin))
		{
			size_t len = strlen(cmd) - 1; // cut off new line character
			bool found = false;
			for(auto &i : commands_)
			{
				if (len == i.first.size() && !strncmp(i.first.c_str(), cmd, i.first.size()))
				{
					found = true;
					i.second.second();
				}
			}
			if (!found)
			{
				print("Unknown command: %s\n", cmd);
				usage();
			}
		} else {
			quit();
		}
	}

private:
	bool quit_;
	std::string buf_;
	std::map<std::string, std::pair<std::string, std::function<void()> > > commands_;

	void quit()
	{
		print("Exiting...\n");
		quit_ = true;
	}

	void in()
	{
		print("Enter text: ");
		char cmd[100];
		if (fgets(cmd, sizeof(cmd), stdin))
		{
			buf_ = std::string(cmd);
		}
	}

	void out()
	{
		print("\"%.*s\"\n", (int)buf_.size() - 1, buf_.c_str());
	}
};

int main()
{
	TestApp app;
	app.usage();

	while(!app)
	{
		app.get_command();
	}
	return 0;
}
