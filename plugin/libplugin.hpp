#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>


#if defined DLL_EXPORT
	#define DllExport __attribute__((visibility ("default")))
#else
	#define DllExport
#endif

#define STRINGIFY(s) QUOTIFY(s)
#define QUOTIFY(s) #s

// Plugin identity function name
#define PLUGIN_ID plugin_id
#define PLUGIN_ID_STR STRINGIFY(PLUGIN_ID)
// Plugin instance function name
#define PLUGIN_INSTANCE plugin_instance
#define PLUGIN_INSTANCE_STR STRINGIFY(PLUGIN_INSTANCE)


/**
 * Plugin definition macro
 * Sample:
 *     DEFINE_PLUGIN_ENTRY( MyAppPlugin );
 */
#define DEFINE_PLUGIN_ENTRY( PLUGIN, NAME, NAMESPACE, VERSION ) \
extern "C" { \
DllExport plugin::Identity PLUGIN_ID() \
{ \
	return plugin::Identity( NAMESPACE, NAME, VERSION ); \
} \
DllExport std::shared_ptr<plugin::Instance> PLUGIN_INSTANCE() \
{ \
	return std::shared_ptr<plugin::Instance>( new PLUGIN() ); \
} \
}

namespace plugin
{

/**
 * Plugin version type
 * Consist of major, minor and patch
 */
class DllExport Version
{
public:
	int major() const;
	int minor() const;
	int patch() const;

	Version( int major, int minor, int patch );
	Version( const Version &rhs );
	bool operator==( const Version &rhs ) const;
	bool operator!=( const Version &rhs ) const;
	bool operator<( const Version &rhs ) const;
	bool operator>( const Version &rhs ) const;
	Version& operator=( const Version &rhs );

	std::string to_string() const;

private:
	int major_;
	int minor_;
	int patch_;
};

/**
 * Plugin identity type
 * - name: plugin name
 * - name_space: plugin space (UI, feature, etc.)
 * - version: plugin release version
 */
class DllExport Identity
{
public:
	std::string name;
	std::string name_space;
	Version version;
	std::string path;

	Identity( const std::string &name_space, const std::string &name, const Version& version );
	Identity( const Identity &rhs );
	bool operator==( const Identity &rhs ) const;
	bool operator!=( const Identity &rhs ) const;
	bool operator<( const Identity &rhs ) const;
	Identity& operator=( const Identity &rhs );

	/*
	 * @return Plugin identity as "<name space>:<name> <version>"
	 */
	std::string to_string() const;
};

/**
 * Plugin instance base class
 */
class DllExport Instance
{
public:
	Instance() : handle_( nullptr )
	{};
	virtual ~Instance()
	{};
	virtual Identity id() const = 0;

private:
	friend class Loader;
	void *handle_;
};


/**
 * Plugin loader class
 */
class DllExport Loader
{
public:
	struct Iterator
	{
		Iterator( const std::string &path, const std::string &name_space, bool end = false );
		~Iterator();
		Iterator& operator++();
		Iterator& operator++( int );
		bool operator==( const Iterator &rhs ) const;
		bool operator!=( const Iterator &rhs ) const;
		const Identity operator*() const;
		const Identity* operator->() const;

	private:
		struct Private;
		std::unique_ptr<Private> data_;

		void next();
	};

	Loader();
	~Loader();

	/**
	 * Search for plugins in a specified directory
	 * @param[in] path - search location
	 * @param[in] name_space - filter plugins of specific name spaces only
	 */
	Iterator begin( const std::string &path, const std::string &name_space = "" ) const;
	Iterator end() const;

	/**
	 * Load plugin
	 * @param[in] id - plugin identity
	 * @return Plugin instance pointer or nullptr if failed to load
	 */
	template<typename Plugin>
	std::weak_ptr<Plugin> load( const Identity &id )
	{
		static_assert( std::is_base_of<Instance, Plugin>::value, "Extension must be base of Instance class" );
		return std::shared_ptr<Plugin>( std::static_pointer_cast<Plugin>( import( id, "" ) ) );
	}

	/**
	 * Load plugin
	 * @param[in] id - plugin identity
	 * @param[in] path - plugin location (if empty, working directory is assumed)
	 * @return Plugin instance pointer or nullptr if failed to load
	 */
	template<typename Plugin>
	std::weak_ptr<Plugin> load( const Identity &id, const std::string &path )
	{
		static_assert( std::is_base_of<Instance, Plugin>::value, "Extension must be base of Instance class" );
		return std::shared_ptr<Plugin>( std::static_pointer_cast<Plugin>( import( id, path ) ) );
	}

	/**
	 * Check if plugin loaded
	 * @param[in] id - plugin identity
	 * @return True if plugin is already loaded
	 */
	bool is_loaded( const Identity &id ) const;

	/**
	 * Unload plugin by it's identity
	 * @param[in] id - plugin identity
	 */
	void unload( const Identity &id );

	/**
	 * Unload all plugins instantiated by the loader instance
	 */
	void unload();

	/**
	 * Get all loaded plugins
	 * @return plugin map
	 */
	const std::map<Identity, std::shared_ptr<Instance> >& plugins() const;

private:
	mutable std::string current_path_;
	std::map<Identity, std::shared_ptr<Instance> > plugins_;

	std::shared_ptr<Instance> import( const Identity &id, const std::string &path );
};

} // namespace plugin
