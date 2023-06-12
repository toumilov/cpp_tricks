
#include <stdio.h>
#include <memory>
#include <functional>


void foo()
{
	std::unique_ptr<int,std::function<void(int*)>> guard( new int, [](int *p){
		printf( "end\n" );
		delete( p );
	});
	printf( "working...\n" );
}

int main()
{
	printf( "start\n" );
	foo();
	printf( "done\n" );

	return 0;
}
