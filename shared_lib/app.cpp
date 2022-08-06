
#include <cstdio>
#include <dlfcn.h>
#include "libtest.hpp"


class Test
{
public:
	static Test& instance()
	{
		static Test inst;
		return inst;
	}

	int get()
	{
		if (get_)
		{
			return get_();
		}
		return 0;
	}

	void set(int n)
	{
		if (set_)
		{
			set_(n);
		}
	}

private:
	void *handle_;
	int (*get_)();
	void (*set_)(int);

	Test() :
		handle_(NULL),
		get_(NULL),
		set_(NULL)
	{
		handle_ = dlopen("libtest.so", RTLD_LAZY);
		if (handle_)
		{
			get_ = (int(*)()) dlsym(handle_, "get");
			set_ = (void(*)(int)) dlsym(handle_, "set2");
		} else {
			fprintf(stderr, "%s\n", dlerror());
		}
	}
};

int main()
{
	set(1);
	printf("Set 1: <%d:%d> %s\n", get(), Test::instance().get(), (get() == 11) ? "OK" : "NOK");
	Test::instance().set(2);
	printf("Set 2(dlopen)<%d:%d> %s\n", get(), Test::instance().get(), (get() == 22) ? "OK" : "NOK");

	return 0;
}
