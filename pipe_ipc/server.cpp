
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <fstream>


int main()
{
	std::string pipe_name( "/tmp/test_fifo" );
	if ( mkfifo( pipe_name.c_str(), 0666 ) )
	{
		fprintf( stderr, "Failed to open pipe\n" );
		return 1;
	}
	std::ifstream ifs( pipe_name );
	// Keep one writing end to prevent EOF
	std::ofstream ofs( pipe_name );
	// Read and process commands
	std::string line;
	while( std::getline( ifs, line ) )
	{
		printf( "<%s>\n", line.c_str() );
		if ( line == "quit" )
		{
			break;
		}
	}
	// Remove socket fs entry
	unlink( pipe_name.c_str() );
	return 0;
}
