
#include <fstream>


int main( int argc, const char *argv[] )
{
	if ( argc < 2 )
	{
		return 0;
	}
	std::ofstream ofs( "/tmp/test_fifo" );
	std::string line( argv[1] );
	for( int i = 2; i < argc; i++ )
	{
		line += " ";
		line += argv[i];
	}
	ofs << line << "\n";
	return 0;
}
