
#include "libtest.hpp"


int global_variable;

#ifdef __cplusplus
extern "C"
{
#endif

int get()
{
	return global_variable;
}

void set(int n)
{
	global_variable = n + 10;
}

void set2(int n)
{
	global_variable = n + 20;
}

#ifdef __cplusplus
}
#endif
