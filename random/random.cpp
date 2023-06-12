#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

bool get_random_data( unsigned char *buf, size_t size )
{
	if ( buf == nullptr || size == 0 )
	{
		return false;
	}
	int fd = open( "/dev/random", O_RDONLY );
	if ( fd == -1 )
	{
		return false;
	}
	bool ret = read( fd, (void*)buf, size ) == (ssize_t)size;
	close( fd );
	return ret;
}

int main()
{
	unsigned u;
	if ( get_random_data( (unsigned char*)&u, sizeof( u ) ) )
	{
		printf( "%08X\n", u );
	} else {
		printf( "Failed to generate random data\n" );
	}
	return 0;
}
