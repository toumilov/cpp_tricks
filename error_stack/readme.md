# Error stack

To provide more detailed error information than just an error code, Error class can be used. Apart from error status code it may hold an error tag (facility) and text description.

Also, a set of errors is very handy to use instead of last error status code. This allows to do a deeper root cause analysis.

# Example
1) Create an error object
```
auto e1 = Error( Error::Failure );
auto e2 = Error( Error::Failure, "Error format check %d", 1 );
Error e3( TestError::Read, "no data" );
```
2) Get error elements
```
assert( e.code() == Error::Failure );
assert( e.tag() == error_tag( Error::Failure ) );
assert( e.description() == "Error format check 1" );
```
3) Custom error type definition
```
class TestError : public Error
{
public:
	enum Code
	{
		Open = 1,
		Read,
		Write,
		Close,
	};
};

std::string error_tag( const TestError::Code& )
{
	return "Test";
}
```
4) Put new error items into stack
```
ErrorStack errs;
errs.push( Error( Error::Failure ) );
errs.push( Error( TestError::Read, "no data" ) );
```
5) Dump error stack to string
```
assert( errs.to_string() == "[1] Generic error (1) \n[2] Test (2) no data" );
```
