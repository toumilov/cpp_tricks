
#include <stdio.h>
#include <scope_exit.hpp>

int main()
{
	printf( "start\n" );
	auto guard = on_scope_exit( [](){ printf( "end\n" ); } );
	auto guard_capital = on_scope_exit( [](){ printf( "END\n" ); } );

	printf( "working...\n" );
	guard_capital.release();

	return 0;
}
