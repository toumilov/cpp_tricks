
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>


int main()
{
	std::string line;
	while( std::getline( std::cin, line ) )
	{
		printf( "<%s>\n", line.c_str() );
	}
	return 0;
}
