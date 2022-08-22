
#include <stdio.h>
#include <unistd.h>
#include <functional>
#include "thread_pool.hpp"


void foo( int i, std::string &&s )
{
	printf("<%d: %s>\n", i, s.c_str());
	sleep(1);
}

class Test
{
public:
	void foo( int i, std::string &&s )
	{
		printf("<%d: %s>\n", i, s.c_str());
		sleep(1);
	}
	static void test( int i, std::string &&s )
	{
		printf("<%d: %s>\n", i, s.c_str());
		sleep(1);
	}
};

int main()
{
	ThreadPool pool( 3 );
	Test t;

	printf( "===foo===\n" );
	for( int i = 0; i < 10; i++ )
	{
		pool.get().run( &foo, int( i ), std::move( std::string( "foo" ) ) );
	}
	printf( "===bind foo===\n" );
	for( int i = 0; i < 10; i++ )
	{
		pool.get().run( std::bind( foo, std::placeholders::_1, std::placeholders::_2 ), int( i ), std::move( std::string( "bind foo" ) ) );
	}
	printf( "===Test::test===\n" );
	for( int i = 0; i < 10; i++ )
	{
		pool.get().run( &Test::test, int( i ), std::move( std::string( "Test::test" ) ) );
	}
	printf( "===bind Test::foo===\n" );
	for( int i = 0; i < 10; i++ )
	{
		pool.get().run( std::bind( &Test::foo, t, std::placeholders::_1, std::placeholders::_2 ), int( i ), std::move( std::string( "bind Test::foo" ) ) );
	}
	printf( "===bind Test::test===\n" );
	for( int i = 0; i < 10; i++ )
	{
		pool.get().run( std::bind( &Test::test, std::placeholders::_1, std::placeholders::_2 ), int( i ), std::move( std::string( "bind Test::test" ) ) );
	}

	printf( "joining...\n" );
	pool.join();

	return 0;
}
