
#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include "thread_pool.hpp"


void test( int i )
{
	printf("<%d>\n", i);
	sleep(1);
}

int main()
{
	ThreadPool pool( 3 );

	for( int i = 0; i < 10; i++ )
	{
		pool.get().run( &test, int( i ) );
	}

	printf( "joining...\n" );
	pool.join();

	return 0;
}
