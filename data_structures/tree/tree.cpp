
#include <initializer_list>
#include <list>
#include <string>
#include <sstream>
#include <algorithm>
#include <set>
#include <stack>
#include <queue>
#include <functional>

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
};

int main()
{
	Element tree;
	tree.add( { "say", "show", "get", "work" } );
	tree.get( "show" )->add( { "usage", "time", "dir" } );
	tree.get( "show/dir" )->add( { "binary", "working" } );
	tree.get( "get" )
		->add( "x" )
			.add( "value" ).parent()
			.add( "type" ).parent().parent()
		.add( "y" )
			.add( "value" ).parent()
			.add( "type" ).parent().parent()
		.add( "z" )
			.add( "value" ).parent()
			.add( "type" ).parent().parent();

	auto show = []( Element &e, uint32_t l ){
		for( int i = 0; i < l; i++ ) printf( "\t" );
		printf( "'%s' [", e.name.c_str() );
		for( const auto &i : e.path() )
		{
			printf( "/%s", i.c_str() );
		}
		printf( "]\n" );
		return true;
	};
	printf( "*************** DFS ****************\n" );
	tree.dfs( show );
	printf( "************* branch ***************\n" );
	tree.get( "show" )->dfs( show );
	printf( "*************** BFS ****************\n" );
	tree.bfs( show );
	printf( "************* branch ***************\n" );
	tree.get( "show" )->bfs( show );
	return 0;
}