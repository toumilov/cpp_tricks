
#include <assert.h>
#include <stdio.h>
#include "error.hpp"


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

int main()
{
	Error e;
	assert( e.empty() );

	e = Error( Error::Failure );
	assert( !e.empty() );
	assert( e.code() == Error::Failure );
	assert( e.tag() == error_tag( Error::Failure ) );

	e = Error( Error::Failure, "Error format check %d", 1 );
	assert( e.description() == "Error format check 1" );
	assert( e.to_string() == "Generic error (1) Error format check 1" );

	Error e2( e );
	assert( e == e2 );
	e2.clear();
	assert( e2.empty() );
	assert( e != e2 );

	e = Error( TestError::Write, "access denied" );
	assert( e.to_string() == "Test (3) access denied" );

	ErrorStack errs;
	assert( errs.empty() );
	assert( errs.size() == 0 );
	errs.push( Error( Error::Failure ) );
	errs.push( Error( TestError::Read, "no data" ) );
	assert( errs.size() == 2 );
	assert( errs.top().code() == TestError::Read );
	assert( errs.to_string() == "[1] Generic error (1) \n[2] Test (2) no data" );
	errs.clear();
	assert( errs.size() == 0 );
	assert( errs.empty() );

	return 0;
}
