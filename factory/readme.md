# Factory pattern C++ implementation

Factory design pattern is widely used for creating object instances based on unique peculiarity.
There could be an outer logic that makes a classification, or it could be generalized based on type specifics (identifier, name, etc.).
Typically, when using IPC, application needs to distinguish a message type, which is going to be handled differently.

## Outer classification
This implementation is designed for identifier-class binding, when id is defined outside of classes.
### Interface
```
struct Factory
{
	template<typename... Args>
	static std::shared_ptr<Base> make_unique( int id, Args... args );
	template<typename... Args>
	static std::shared_ptr<Base> make_shared( int id, Args... args );
	template<typename... Args>
	static Base* make( int id, Args... args )
}
```
### Usage example
```
enum Types : int
{
	TypeA = 0,
	TypeB
};
typedef Factory<Base,
		FactoryElement<TypeA, A>,
		FactoryElement<TypeB, B>
	> TestFactory;
auto shared_a = TestFactory::make_shared( TypeA, "123" );
auto shared_b = TestFactory::make_shared( TypeB, "456" );
```
**Sample:** *basic_factory.cpp*

## Inner classification
This factory implementation has several advenatages:
* Each class defines it's identifier for the factory (static function)
* Compile time identification checking
* Additional visitor implementation for type-specific object ptocessing

### Interface
```
struct Factory
{
	template<typename... Args>
	static std::unique_ptr<Base> make_unique( const IdType &id, Args... args );
	template<typename... Args>
	static std::shared_ptr<Base> make_shared( const IdType &id, Args... args );
	template<typename... Args>
	static Base* make( const IdType &id, Args... args );
	template<typename Visitor>
	static void accept( const IdType &id, Base *item, Visitor &visitor );
}
```

### Usage example
```
typedef Factory<Base, A, B> TestFactory;
struct ControllerVisitor
{
	void operator()() // Default handler
	{
		std::cout << "Default handler";
	}
	void operator()( A &a )
	{
		std::cout << "A handler";
	}
	void operator()( B &b )
	{
		std::cout << "B handler";
	}
};
ControllerVisitor ctrl;
TestFactory::accept( "a", shared_a.get(), ctrl );
```

**Sample:** *factory.cpp*
