#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <mutex>

// Console tracing helpers
#define log_error( format, ... )	Trace::instance().msg( __FILE__, __LINE__, Trace::Level::Error, (format), ## __VA_ARGS__ )
#define log_debug( format, ... )	Trace::instance().msg( __FILE__, __LINE__, Trace::Level::Debug, (format), ## __VA_ARGS__ )

// Tracing implementation (reduced to error and debug levels only)
class Trace
{
public:
	enum class Level
	{
		Error,
		Debug
	};

	static Trace& instance();
	void setup( Level lvl );
	void msg( const char *file, unsigned line, Level level, const char *format, ... )
			__attribute__ ((format (printf, 5, 6)));

private:
	Trace();
	std::mutex mtx_;
	Level level_;
};

// Base class for user query logging
struct LoggerBase
{
	LoggerBase() {};
	virtual ~LoggerBase() {};
	virtual void log( const std::string &s ) = 0;
};

// User SQL request logger to text file
struct FileLogger : LoggerBase
{
	/* RAII c-tor
	 * @param[in] file_name - name of the output file (relative to current working directory)
	 */
	FileLogger( const std::string &file_name );
	~FileLogger();
	/*
	 * @param[in] query - SQL query text to save into log
	 */
	virtual void log( const std::string &query ) override;
private:
	std::ofstream fs;
};
