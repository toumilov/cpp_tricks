#include <cstdio>
#include <ctime>
#include <stdarg.h>
#include "logger.hpp"

// Printf-like format function which returns std::string
static std::string __attribute__((format (printf, 1, 2))) string_format( const char *format, ... )
{
	va_list vl, vl2;
	va_start( vl, format );
	va_copy( vl2, vl );
	auto size = vsnprintf( NULL, 0, format, vl );
	va_end( vl );
	std::string s;
	if ( size <= 0 )
	{
		return s;
	}
	s.resize( size );
	vsnprintf( (char*)s.data(), s.size() + 1, format, vl2 );
	va_end( vl2 );
	return s;
}

static const char* to_string( Trace::Level level )
{
	switch( level )
	{
		case Trace::Level::Error:
			return "error";
		case Trace::Level::Debug:
			return "debug";
		default:
			break;
	}
	return "off";
}

Trace::Trace() :
	level_( Level::Error )
{}

Trace& Trace::instance()
{
	static Trace inst;
	return inst;
}

void Trace::setup( Level lvl )
{
	level_ = lvl;
}

void Trace::msg( const char *file, unsigned line, Level level, const char *format, ... )
{
	if ( level_ < level )
	{
		return;
	}

	// Format message
	static const size_t date_time_size = 25;
	char date_time[date_time_size];
	time_t t = std::time( nullptr );
	struct tm time_stamp;
	strftime( date_time, date_time_size, "%F %T", localtime_r( &t, &time_stamp ) );
	auto message = string_format( "%s [%s](%s:%u) ", date_time, to_string( level ), file, line );

	va_list vl, vl2;
	va_start( vl, format );
	va_copy( vl2, vl );
	auto size = vsnprintf( NULL, 0, format, vl );
	va_end( vl );
	std::string str;
	if ( size <= 0 )
	{
		return;
	}
	str.resize( size );
	vsnprintf( (char*)str.data(), str.size() + 1, format, vl2 );
	va_end( vl2 );

	message += str;

	// Acquire lock
	std::lock_guard<std::mutex> lck( mtx_ );

	// Log to console
	fprintf( stderr, "%s\n", message.c_str() );
}


FileLogger::FileLogger( const std::string &file_name )
{
	const auto out_path = std::filesystem::current_path() / file_name;
	fs.open( out_path.c_str(), std::ofstream::out | std::ofstream::trunc );
}

FileLogger::~FileLogger()
{
	fs.flush();
	fs.close();
}

void FileLogger::log( const std::string &s )
{
	fs << s << std::endl;
}
