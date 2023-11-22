#include <cstring>
#include <cstdio>
#include <initializer_list>
#include <list>
#include <string>
#include <sstream>
#include <algorithm>
#include <set>
#include <stack>
#include <queue>
#include <functional>


struct Autocomplete
{
	struct Element
	{
		std::string name;
		std::set<Element> children;

		Element() :
			backlink_( nullptr )
		{}
		Element( const std::string &name ) :
			name( name ),
			backlink_( nullptr )
		{}
		Element( const std::string &name, Element *backlink ) :
			name( name ),
			backlink_( backlink )
		{}
		Element( const Element &rhs ) :
			name( rhs.name ),
			children( rhs.children ),
			backlink_( rhs.backlink_ )
		{}
		Element( const Element &&rhs ) :
			name( std::move( rhs.name ) ),
			children( std::move( rhs.children ) ),
			backlink_( std::move( rhs.backlink_ ) )
		{}
		Element& operator=( Element &rhs )
		{
			name = rhs.name;
			children = rhs.children;
			backlink_ = rhs.backlink_;
			return *this;
		}
		bool operator<( const Element &rhs ) const
		{
			return name < rhs.name;
		}
		Element& add( const std::string &name )
		{
			auto [item, inserted] = children.emplace( name, this );
			return const_cast<Element&>( *item );
		}
		Element& add( std::initializer_list<std::string> items )
		{
			for( const auto &i : items )
			{
				children.emplace( i, this );
			}
			return *this;
		}
		Element& parent()
		{
			return *backlink_;
		}
		Element* get( const char *path )
		{
			Element *ptr = this;
			std::stringstream stream( path );
			std::string item;
			auto pred = [&item](const Element &e){ return item == e.name; };
			while( getline( stream, item, '/' ) )
			{
				if ( item.empty() )
				{
					continue;
				}
				auto it = std::find_if( ptr->children.begin(), ptr->children.end(), pred );
				if ( it == ptr->children.end() )
				{
					return nullptr;
				}
				ptr = const_cast<Element*>( &(*it) ); 
			}
			return ptr;
		}
		std::list<std::string> path() const
		{
			std::list<std::string> path;
			const Element *ptr = this; 
			while( ptr->backlink_ )
			{
				path.push_front( ptr->name );
				ptr = ptr->backlink_;
			}
			return path;
		}
		typedef std::function<bool(Element&, uint32_t)> TraversalCallback;
		void dfs( TraversalCallback cb )
		{
			static Element *separator = reinterpret_cast<Element*>( 0x1 );
			std::stack<Element*> elements;
			uint32_t depth = 0;
			if ( !name.empty() )
			{
				if ( !cb( *this, depth ) )
				{
					return;
				}
				depth++;
			}
			elements.push( separator );
			for( auto it = children.rbegin(); it != children.rend(); ++it )
			{
				elements.push( const_cast<Element*>( &(*it) ) );
			}
			while( !elements.empty() )
			{
				auto top = elements.top();
				elements.pop();
				if ( top == separator )
				{
					depth--;
					continue;
				}
				if ( !cb( *top, depth ) )
				{
					break;
				}
				if ( !top->children.empty() )
				{
					depth++;
					elements.push( separator );
					for( auto it = top->children.rbegin(); it != top->children.rend(); ++it )
					{
						elements.push( const_cast<Element*>( &(*it) ) );
					}
				}
			}
		}
		void bfs( TraversalCallback cb )
		{
			static Element *separator = reinterpret_cast<Element*>( 0x1 );
			uint32_t depth = 0;
			if ( !name.empty() )
			{
				if ( !cb( *this, depth ) )
				{
					return;
				}
				depth++;
			}
			std::queue<Element*> elements;
			for( auto &i : children )
			{
				elements.push( const_cast<Element*>( &i ) );
			}
			elements.push( separator );
			while( true )
			{
				auto top = elements.front();
				elements.pop();
				if ( top == separator )
				{
					if ( elements.empty() )
					{
						break;
					}
					depth++;
					elements.push( separator );
					continue;
				}
				if ( !cb( *top, depth ) )
				{
					break;
				}
				if ( !top->children.empty() )
				{
					for( auto &i : top->children )
					{
						elements.push( const_cast<Element*>( &i ) );
					}
				}
			}
		}
	private:
		Element *backlink_;
	} elements;

	Autocomplete( int argc, char **argv ) :
		argc_( argc ),
		argv_( argv )
	{}

	static bool is_autocomplete()
	{
		auto *comp_type = std::getenv( "COMP_TYPE" );
		if ( !comp_type )
		{
			return false;
		}
		long type = strtol( comp_type, nullptr, 10 );
		return type == '\t' || type == '?';
	}

	virtual void build() = 0;

	bool autocomplete()
	{
		if ( is_autocomplete() )
		{
			build();

			// Get base
			std::string comp_line( std::getenv( "COMP_LINE" ) );
			size_t n = comp_line.find_first_not_of( " ", comp_line.find( ' ' ) );
			if ( n == std::string::npos )
			{
				comp_line.clear();
			} else {
				comp_line = comp_line.substr( n );
			}
			n = comp_line.rfind( ' ' );
			std::string complete_args;
			std::string pending_arg;
			if ( n == std::string::npos )
			{
				pending_arg = comp_line.c_str();
			} else {
				complete_args = comp_line.substr( 0, n );
				pending_arg = comp_line.substr( n + 1 );
			}

			// Build path
			std::string arg_path;
			{
				std::string item;
				std::stringstream stream( complete_args );
				while( getline( stream, item, ' ' ) )
				{
					if ( item.empty() )
					{
						continue;
					}
					arg_path += "/";
					arg_path += item;
				}
			}
			auto starts_with = []( const std::string &str, const std::string &beginning ) -> bool {
				return str.find( beginning ) == 0;
			};
			auto e = elements.get( arg_path.c_str() );
			if ( e )
			{
				std::list<std::string> options;
				// Find options starting with pending_arg
				for( const auto &option : e->children )
				{
					if ( starts_with( option.name, pending_arg ) )
					{
						options.push_back( option.name );
						printf( "%s\n", option.name.c_str() );
					}
				}
			}
			return true;
		}
		return false;
	}

protected:
	int argc_;
	char **argv_;
};

struct MyAutocomplete : public Autocomplete
{
	MyAutocomplete( int argc, char **argv ) :
		Autocomplete( argc, argv )
	{}
	void build() override
	{
		elements.add( { "say", "show", "get", "work" } );
		elements.get( "show" )->add( { "usage", "time", "dir" } );
		elements.get( "show/dir" )->add( { "binary", "working" } );
		elements.get( "get" )
			->add( "x" )
				.add( "value" ).parent()
				.add( "type" ).parent().parent()
			.add( "y" )
				.add( "value" ).parent()
				.add( "type" ).parent().parent()
			.add( "z" )
				.add( "value" ).parent()
				.add( "type" ).parent().parent();
	}
};

int main( int argc, char **argv )
{
	// complete -C /usr/bin/autocomplete/ac_sample ac_sample
	MyAutocomplete ac( argc, argv );
	if ( ac.autocomplete() )
	{
		// Autocomplete processed
		return 0;
	}
	printf( "Processing: {" );
	for( int i = 1; i < argc; i++ )
	{
		if ( i > 1 )
		{
			printf( ", " );
		}
		printf( "%s", argv[i] );
	}
	printf( "}\n" );
	return 0;
}