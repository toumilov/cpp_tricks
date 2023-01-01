
#include <dlfcn.h>
#include <filesystem>
#include <map>
#include "libplugin.hpp"

namespace fs = std::filesystem;

namespace plugin
{

/*
 * Version
 */

Version::Version( int major, int minor, int patch ) :
		major_( major ),
		minor_( minor ),
		patch_( patch )
{}

Version::Version( const Version &rhs ) :
		major_( rhs.major_ ),
		minor_( rhs.minor_ ),
		patch_( rhs.patch_ )
{}

bool Version::operator==( const Version &rhs ) const
{
	return major_ == rhs.major_ && minor_ == rhs.minor_ && patch_ == rhs.patch_;
}

bool Version::operator!=( const Version &rhs ) const
{
	return !operator==( rhs );
}

bool Version::operator<( const Version &rhs ) const
{
	if ( major_ < rhs.major_ )
	{
		return true;
	}
	else if ( major_ > rhs.major_ )
	{
		return false;
	}
	if ( minor_ < rhs.minor_ )
	{
		return true;
	}
	else if ( minor_ > rhs.minor_ )
	{
		return false;
	}
	return patch_ < rhs.patch_;
}

bool Version::operator>( const Version &rhs ) const
{
	return operator != ( rhs ) && !operator < (rhs);
}

Version& Version::operator=( const Version &rhs )
{
	major_ = rhs.major_;
	minor_ = rhs.minor_;
	patch_ = rhs.patch_;
	return *this;
}

std::string Version::to_string() const
{
	return std::to_string( major_ ) + "." + std::to_string( minor_ ) + "." + std::to_string( patch_ );
}


/*
 * Identity
 */

Identity::Identity( const std::string &name_space, const std::string &name, const Version& version ) :
		name( name ),
		name_space( name_space ),
		version( version )
{}

Identity::Identity( const Identity &rhs ) :
		name( rhs.name ),
		name_space( rhs.name_space ),
		version( rhs.version ),
		path( rhs.path )
{}

bool Identity::operator==( const Identity &rhs ) const
{
	return name == rhs.name && name_space == rhs.name_space && version == rhs.version;
}

bool Identity::operator!=( const Identity &rhs ) const
{
	return !operator==( rhs );
}

bool Identity::operator<( const Identity &rhs )  const
{
	return name < rhs.name || name_space < rhs.name_space || version < rhs.version;
}

Identity& Identity::operator=( const Identity &rhs )
{
	name = rhs.name;
	name_space = rhs.name_space;
	version = rhs.version;
	path = rhs.path;
	return *this;
}

std::string Identity::to_string() const
{
	return name_space + ":" + name + " " + version.to_string();
}


// Loader::Iterator

struct Loader::Iterator::Private
{
	Private( const fs::path &dir, const std::string &name_space, bool end ) :
		dir( dir ),
		name_space( name_space ),
		it( dir ),
		id( name_space, "", Version( 1, 0, 0 ) )
	{
		if ( end )
		{
			it = fs::end( it );
		}
	}

	~Private()
	{}

	fs::path dir;
	std::string name_space;
	fs::directory_iterator it;
	Identity id;
};

Loader::Iterator::Iterator( const std::string &path, const std::string &name_space, bool end )
{
	const fs::path dir = path.empty() ? fs::current_path() : fs::path( path );
	data_.reset( new Private( dir, name_space, end ) );
	if ( !path.empty() )
	{
		next();
	}
}

Loader::Iterator::~Iterator()
{}

void Loader::Iterator::next()
{
	if ( data_->dir.empty() )
	{
		return;
	}
	for( ;data_->it != fs::end( data_->it ); ++data_->it )
	{
		fs::path entry;
		if ( data_->it->is_regular_file() )
		{
			entry = data_->it->path();
		}
		else if ( data_->it->is_symlink() && data_->it->exists() )
		{
			entry = fs::read_symlink( data_->it->path() );
		}
		else
		{
			continue;
		}
		// Skip files that are not shared objects
		if ( entry.string().find( ".so" ) == std::string::npos )
		{
			continue;
		}

		// Try to open file as a library
		void *handle = dlopen( entry.c_str(), RTLD_NOW );
		if ( handle == nullptr )
		{
			fprintf( stderr, "Failed to open %s: %s\n", entry.c_str(), dlerror() );
			continue;
		}
		bool skip = true;
		do
		{
			// Try to find plugin entry
			Identity(*plugin_id)() = (Identity(*)())dlsym( handle, PLUGIN_ID_STR );
			if ( plugin_id == nullptr )
			{
				// No plugin entry
				break;
			}
			char *error = dlerror();
			if ( error != nullptr )
			{
				fprintf( stderr, "Plugin load error: %s\n", error );
				break;
			}
			auto id = plugin_id();
			if ( !data_->name_space.empty() && id.name_space != data_->name_space )
			{
				// Not the namespace we are looking for
				break;
			}
			// OK, we've got one!
			data_->id = id;
			data_->id.path = entry.string();
			skip = false;
		} while( false );

		dlclose( handle );

		if ( !skip )
		{
			break;
		}
	}
}

Loader::Iterator& Loader::Iterator::operator++()
{
	if ( !data_->dir.empty() && data_->it != fs::end( data_->it ) )
	{
		if ( data_->it != fs::end( data_->it ) )
		{
			++data_->it;
		}
		next();
	}
	return *this;
}

Loader::Iterator& Loader::Iterator::operator++( int )
{
	return operator ++();
}

bool Loader::Iterator::operator==( const Iterator &rhs ) const
{
	return data_->it == rhs.data_->it;
}

bool Loader::Iterator::operator!=( const Iterator &rhs ) const
{
	return !operator==( rhs );
}

const Identity Loader::Iterator::operator*() const
{
	return data_->id;
}

const Identity* Loader::Iterator::operator->() const
{
	return &data_->id;
}


// Loader

Loader::Loader()
{}

Loader::~Loader()
{
	unload();
}

Loader::Iterator Loader::begin( const std::string &path, const std::string &name_space ) const
{
	current_path_ = path;
	return Iterator( path, name_space );
}

Loader::Iterator Loader::end() const
{
	return Iterator( current_path_, "", true );
}

const std::map<Identity, std::shared_ptr<Instance> >& Loader::plugins() const
{
	return plugins_;
}

std::shared_ptr<Instance> Loader::import( const Identity &id, const std::string &path )
{
	std::string id_path = id.path;

	// If identity path is empty (not found by iterator) - find it
	if ( id_path.empty() )
	{
		fs::path dir;
		if ( path.empty() )
		{
			// Search in working directory
			dir = fs::current_path();
		} else {
			// Search in specified path
			dir = fs::path( path );
		}
		for( auto it = begin( dir ); it != end(); ++it )
		{
			if ( *it == id )
			{
				id_path = it->path;
				break;
			}
		}
		if ( id_path.empty() )
		{
			return nullptr;
		}
	}

	if ( is_loaded( id ) )
	{
		return nullptr; // Already loaded
	}

	// Load by identity

	std::error_code ec;
	if ( !fs::exists( id_path, ec ) )
	{
		return nullptr;
	}

	// Try to open file as a library
	void *handle = dlopen( id_path.c_str(), RTLD_NOW );
	if ( handle == nullptr )
	{
		fprintf( stderr, "Failed to open %s: %s\n", id_path.c_str(), dlerror() );
		return nullptr;
	}

	std::shared_ptr<Instance> inst;

	do
	{
		// Try to find plugin entry
		Identity(*plugin_id)() = (Identity(*)())dlsym( handle, PLUGIN_ID_STR );
		if ( plugin_id == nullptr )
		{
			// No plugin entry
			break;
		}
		char *error = dlerror();
		if ( error != nullptr )
		{
			fprintf( stderr, "Plugin load error: %s\n", error );
			return nullptr;
		}
		auto lib_id = plugin_id();
		if ( lib_id != id )
		{
			fprintf( stderr, "Wrong identity\n" );
			break;
		}

		// OK, we've found right identity

		// Try to get plugin instance entry
		std::shared_ptr<Instance>(*plugin_instance)() = (std::shared_ptr<Instance>(*)())dlsym( handle, PLUGIN_INSTANCE_STR );
		if ( plugin_instance == nullptr )
		{
			// No plugin entry
			break;
		}
		error = dlerror();
		if ( error != nullptr )
		{
			fprintf( stderr, "Plugin instance load error: %s\n", error );
			break;
		}

		inst = plugin_instance();
		if ( !inst )
		{
			fprintf( stderr, "Plugin instantiation error\n" );
			break;
		}

		inst->handle_ = handle;

		// Put instance to the map
		plugins_.insert( std::make_pair( id, inst ) );
		handle = nullptr;

	} while( false );

	if ( handle )
	{
		dlclose( handle );
	}

	return inst;
}

bool Loader::is_loaded( const Identity &id ) const
{
	return plugins_.find( id ) != plugins_.end();
}

void Loader::unload( const Identity &id )
{
	auto it = plugins_.find( id );
	if ( it != plugins_.end() )
	{
		void *handle = it->second->handle_;
		plugins_.erase( it );
		dlclose( handle );
	}
}

void Loader::unload()
{
	for( auto it = plugins_.begin(); it != plugins_.end(); )
	{
		void *handle = it->second->handle_;
		it = plugins_.erase( it );
		dlclose( handle );
	}
}

} // namespace plugin
