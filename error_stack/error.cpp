
#include "error.hpp"


/*
 * Error
 */
Error::Error() :
		code_( Code::None )
{}

Error::Error( const Code code ) :
		code_( code ),
		tag_( error_tag( code ) )
{}

Error::Error( unsigned code, const std::string &tag, const std::string &desc ) :
		code_( code ),
		tag_( tag ),
		description_( desc )
{}

Error& Error::operator=( const Error &e )
{
	code_ = e.code_;
	tag_ = e.tag_;
	description_ = e.description_;
	return *this;
}

bool Error::operator==( const Error &e ) const
{
	return code_ == e.code_ && tag_ == e.tag_;
}

bool Error::operator!=( const Error &e ) const
{
	return !operator ==( e );
}

unsigned Error::code() const
{
	return code_;
}

const std::string& Error::description() const
{
	return description_;
}

const std::string& Error::tag() const
{
	return tag_;
}

void Error::tag( const std::string &t )
{
	tag_ = t;
}

bool Error::empty() const
{
	return code_ == Code::None;
}

void Error::clear()
{
	code_ = Code::None;
	tag_.clear();
	description_.clear();
}

std::string Error::to_string() const
{
	// Generic error [1] description
	return std::string( tag_ ) + " (" + std::to_string( code_ ) + ") " + description_;
}

std::string error_tag( const Error::Code& )
{
	return "Generic error";
};

/*
 * ErrorStack
 */
ErrorStack::ErrorStack()
{}

ErrorStack::ErrorStack( const ErrorStack &e ) :
		items_( e.items_ )
{
}

ErrorStack::ErrorStack( const ErrorStack &&e ) :
		items_( std::move( e.items_ ) )
{
}

ErrorStack& ErrorStack::operator=( const ErrorStack &e )
{
	items_ = e.items_;
	return *this;
}

ErrorStack& ErrorStack::operator=( const ErrorStack &&e )
{
	items_ = std::move( e.items_ );
	return *this;
}

void ErrorStack::swap( ErrorStack &e )
{
	items_.swap( e.items_ );
}

bool ErrorStack::empty() const
{
	return items_.empty();
}

size_t ErrorStack::size() const
{
	return items_.size();
}

void ErrorStack::push( const Error &e )
{
	items_.push_back( e );
}

void ErrorStack::push( const Error &&e )
{
	items_.push_back( std::move( e ) );
}

const Error& ErrorStack::top() const
{
	static Error e;
	if ( items_.empty() )
	{
		return e;
	}
	return items_.back();
}

void ErrorStack::clear()
{
	items_.clear();
}

const std::list<Error> ErrorStack::items() const
{
	return items_;
}

std::string ErrorStack::to_string() const
{
	std::string out;
	std::list<Error> elements( items_ );
	int n = 1;
	for( auto i = items_.begin(); i != items_.end(); ++i )
	{
		if ( !out.empty() )
		{
			out += "\n";
		}
		std::string l( "[" );
		l += std::to_string( n );
		l += "] ";
		l += i->to_string();
		n++;
		out += l;
	}
	return out;
}
