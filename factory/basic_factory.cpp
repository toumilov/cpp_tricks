
#include <iostream>
#include <string>
#include <memory>
#include <type_traits>

template<int Id, typename T>
struct FactoryElement {};

template<typename Base, typename...>
struct Factory;

template<typename Base, int... Ids, typename... Types>
struct Factory<Base, FactoryElement<Ids, Types>...>
{
	template<typename... Args>
	static std::shared_ptr<Base> make_unique( int id, Args... args )
	{
		return make<std::unique_ptr<Base> >( id, std::forward<Args>( args )... );
	}

	template<typename... Args>
	static std::shared_ptr<Base> make_shared( int id, Args... args )
	{
		return make<std::shared_ptr<Base> >( id, std::forward<Args>( args )... );
	}

	template<typename BP = Base*, typename... Args>
	static BP make( int id, Args... args )
	{
		int items[] = { Ids... };
		Base* (*ctors[])( Args... ) = { &construct<Types>... };
		for( size_t i = 0; i < sizeof...(Ids); ++i )
		{
			if ( id == items[i] )
			{
				return BP( (*ctors[i])( std::forward<Args>( args )... ) );
			}
		}
		return nullptr;
	}

private:
	template<typename T, typename... Args>
	static Base* construct( Args... args )
	{
		return new T( std::forward<Args>( args )... );
	}
};


enum Types : int
{
	TypeA = 0,
	TypeB
};

struct Base
{
	Base( const char *arg ) :
		data_( arg )
	{}
	virtual ~Base()
	{}
	virtual std::string status() const = 0;

protected:
	std::string data_;
};

struct A : Base
{
	A( const char *arg ) :
		Base( arg )
	{}
	virtual std::string status() const
	{
		return std::string( "A(" ) + data_ + ")";
	}
};

struct B : Base
{
	B( const char *arg ) :
		Base( arg )
	{}
	virtual std::string status() const
	{
		return std::string( "B(" ) + data_ + ")";
	}
};

typedef Factory<Base,
		FactoryElement<TypeA, A>,
		FactoryElement<TypeB, B>
	> TestFactory;

int main()
{
	static_assert(std::is_same<decltype(&A::status), std::string(A::*)() const>::value, "A");
	static_assert(std::is_same<decltype(&B::status), std::string(B::*)() const>::value, "B");

	auto shared_a = TestFactory::make_shared( TypeA, "123" );
	auto shared_b = TestFactory::make_shared( TypeB, "456" );
	std::cout << shared_a->status().c_str() << std::endl;
	std::cout << shared_b->status().c_str() << std::endl;

	return 0;
}
