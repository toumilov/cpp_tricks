
#include <iostream>
#include <string>
#include <memory>
#include <type_traits>


template<typename Base, class... Types>
struct Factory
{
	typedef decltype(std::declval<Base>().id()) IdType;

	template<typename... Args>
	static std::unique_ptr<Base> make_unique( const IdType &id, Args... args )
	{
		return make<std::unique_ptr<Base> >( id, std::forward<Args>( args )... );
	}

	template<typename... Args>
	static std::shared_ptr<Base> make_shared( const IdType &id, Args... args )
	{
		return make<std::shared_ptr<Base> >( id, std::forward<Args>( args )... );
	}

	template<typename BP = Base*, typename... Args>
	static BP make( const IdType &id, Args... args )
	{
		IdType (*ids[])() = { &identify<Types>... };
		Base* (*ctors[])( Args... ) = { &construct<Types>... };
		for( size_t i = 0; i < sizeof...(Types); ++i )
		{
			if ( id == (*ids[i])() )
			{
				return BP( (*ctors[i])( std::forward<Args>( args )... ) );
			}
		}
		return nullptr;
	}

	template<typename BP = Base*, typename Visitor>
	static void accept( const IdType &id, Base *item, Visitor &visitor )
	{
		IdType (*ids[])() = { &identify<Types>... };
		void (*dispatchers[])( BP, Visitor& ) = { &dispatch<Types>... };
		for( size_t i = 0; i < sizeof...(Types); ++i )
		{
			if ( id == (*ids[i])() )
			{
				return (*dispatchers[i])( item, visitor );
			}
		}
		visitor.operator()();
	}

private:
	template<typename T>
	static IdType identify()
	{
		static_assert(std::is_same<decltype(T::id()), IdType>::value, "Factory item must identify itself");
		return T::id();
	}
	template<typename T, typename... Args>
	static Base* construct( Args... args )
	{
		return new T( std::forward<Args>( args )... );
	}
	template<typename T, typename Visitor>
	static void dispatch( Base *item, Visitor &v )
	{
		v( static_cast<T&>( *item ) );
	}
};


struct Base
{
	std::string arg;

	Base( const char *arg ) :
		arg( arg )
	{}
	virtual ~Base()
	{}
	virtual bool parse( const std::string &data ) const = 0;
	static std::string id() // dummy
	{
		return "";
	}
};

struct A : Base
{
	A( const char *arg ) :
		Base( arg )
	{}

	static std::string id()
	{
		return "a";
	}
	virtual bool parse( const std::string &data ) const
	{
		std::cout << "A[" << arg << "]::parse(" << data << ")" << std::endl;
		return true;
	}
};

struct B : Base
{
	B( const char *arg ) :
		Base( arg )
	{}

	static std::string id()
	{
		return "b";
	}
	virtual bool parse( const std::string &data ) const
	{
		std::cout << "B[" << arg << "]::parse(" << data << ")" << std::endl;
		return true;
	}
};

typedef Factory<Base, A, B> TestFactory;

struct ControllerVisitor
{
	std::string data;

	void operator()() // Default handler
	{
		data = "default";
		std::cout << "Default handler";
	}
	void operator()( A &a )
	{
		data = a.arg;
		std::cout << "A handler";
	}
	void operator()( B &b )
	{
		data = b.arg;
		std::cout << "B handler";
	}
};

int main()
{
	// Construct instances based on type identifier
	auto shared_a = TestFactory::make_shared( "a", "A argument" );
	auto shared_b = TestFactory::make_shared( "b", "B argument" );

	// Call method
	shared_a->parse( "123" );
	shared_b->parse( "456" );

	// Process object according to it's type
	ControllerVisitor ctrl;

	std::cout << "shared_a: ";
	TestFactory::accept( "a", shared_a.get(), ctrl );
	std::cout << " => " << ctrl.data << std::endl;

	std::cout << "shared_b: ";
	TestFactory::accept( "b", shared_b.get(), ctrl );
	std::cout << " => " << ctrl.data << std::endl;

	std::cout << "unknown: ";
	TestFactory::accept( "c", nullptr, ctrl );
	std::cout << " => " << ctrl.data << std::endl;

	return 0;
}
