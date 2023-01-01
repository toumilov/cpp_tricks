
#include "cstdio"
#include "my_plugin.hpp"


DEFINE_PLUGIN_ENTRY( MyAppPlugin,
		MyAppPlugin::name(),
		MyAppPlugin::name_space(),
		MyAppPlugin::version() );

MyAppPlugin::MyAppPlugin()
{
	printf( "MyAppPlugin()\n" );
}

MyAppPlugin::~MyAppPlugin()
{
	printf( "~MyAppPlugin()\n" );
}

const char* MyAppPlugin::name()
{
	return "MyAppPlugin";
}

const char* MyAppPlugin::name_space()
{
	return "MyApp";
}

plugin::Version MyAppPlugin::version()
{
	return plugin::Version( PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH );
}

plugin::Identity MyAppPlugin::id() const
{
	return plugin::Identity( name_space(), name(), version() );
}

std::string MyAppPlugin::foo( int arg ) const
{
	return std::string( "MyAppPlugin.foo(" ) + std::to_string( arg ) + ")";
}
