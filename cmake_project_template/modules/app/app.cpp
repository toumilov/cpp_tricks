#include <stdio.h>
#include "foo.hpp"

int main()
{
#ifdef x86
	printf("Running on x86 device\n");
#endif
	printf("<%d>\n", foo(5));
#ifdef FEATURE_BAR
	printf("<%d>\n", bar(5));
#endif
	return 0;
}