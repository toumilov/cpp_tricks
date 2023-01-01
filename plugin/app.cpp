
#include <cstdio>
#include <filesystem>
#include "libplugin.hpp"
#include "my_plugin.hpp"

namespace fs = std::filesystem;

int main()
{
	// Search for plugins in current directory
	plugin::Loader loader;
	for( auto it = loader.begin( fs::current_path().string() ); it != loader.end(); ++it )
	{
		// Found plugin
		printf( "Found plugin: '%s' [%s]\n", it->to_string().c_str(), it->path.c_str() );
		// Check if it's already loaded
		if ( loader.is_loaded( *it ) )
		{
			printf( "Plugin '%s' is already loaded\n", it->to_string().c_str() );
		} else {
			printf( "Loading plugin '%s'\n", it->to_string().c_str() );
			std::shared_ptr<MyAppPlugin> p = loader.load<MyAppPlugin>( *it ).lock();
			if ( p )
			{
				printf( "Calling MyAppPlugin::foo()\n" );
				printf( "\t<%s>\n", p->foo( 5 ).c_str() );
			} else {
				printf( "Failed to instantiate plugin\n" );
			}
		}
	}
	printf( "Cleanup...\n" );

	return 0;
}
