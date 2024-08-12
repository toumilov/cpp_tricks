#include "foo.hpp"

int foo(int x)
{
	return x * 2;
}

#ifdef FEATURE_BAR
bool bar(int x)
{
	return (x % 2) == 0;
}
#endif