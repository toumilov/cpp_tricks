#pragma once

#include <cstdarg>
#include <string>
#include <list>


class Error
{
public:
	enum Code
	{
		None = 0xFFFFFFFF,
		Failure = 1,
	};

	Error();
	Error( const Code );
	Error( unsigned code, const std::string &tag, const std::string &desc );
	template <class T>
	Error( T e ) :
		code_( e ),
		tag_( error_tag( e ) )
	{}
	template <class T>
	Error( T e, const char *format, ... ) :
		code_( e ),
		tag_( error_tag( e ) )
	{
		va_list vl, vl2;
		va_start( vl, format );
		va_copy( vl2, vl );
		auto size = vsnprintf( NULL, 0, format, vl );
		va_end( vl );
		if ( size > 0 )
		{
			description_.resize( size );
			vsnprintf( (char*)description_.data(), description_.size() + 1, format, vl2 );
		}
		va_end( vl2 );
	}

	Error& operator=( const Error& );
	bool operator==( const Error& ) const;
	bool operator!=( const Error& ) const;

	unsigned code() const;
	const std::string& description() const;
	const std::string& tag() const;
	void tag( const std::string& );
	bool empty() const;
	void clear();
	std::string to_string() const;

protected:
	unsigned code_;
	std::string tag_;
	std::string description_;
};

std::string error_tag( const Error::Code& );

class ErrorStack
{
public:
	ErrorStack();
	ErrorStack( const ErrorStack& );
	ErrorStack( const ErrorStack&& );
	ErrorStack& operator=( const ErrorStack& );
	ErrorStack& operator=( const ErrorStack&& );
	void swap( ErrorStack& );
	bool empty() const;
	size_t size() const;
	void push( const Error& );
	void push( const Error&& );
	const Error& top() const;
	void clear();
	const std::list<Error> items() const;
	std::string to_string() const;

private:
	std::list<Error> items_;
};
