#include <iostream>
#include <system_error>
#include <string>


enum MyErrorCodes : int
{
	OK = 0,
	Client = 1,
	ClientAbort,
	Server = 100,
	ServerBusy,
	Ipc = 200,
	IpcTimeout,
	IpcRead,
	IpcWrite,
	Other = 300
};

namespace std
{
template<> struct is_error_condition_enum<MyErrorCodes> : public true_type {};
}

// custom category:
class MyErrorCategory_t : public std::error_category
{
public:
	virtual const char* name() const noexcept override
	{
		return "My Error";
	}

	virtual std::error_condition default_error_condition( int err_value ) const noexcept override
	{
		MyErrorCodes error_value = static_cast<MyErrorCodes>( err_value );
		if ( error_value == MyErrorCodes::OK )
		{
			return std::error_condition( MyErrorCodes::OK );
		}
		else if ( ( error_value >= MyErrorCodes::Client ) && ( error_value < MyErrorCodes::Server ) )
		{
			return std::error_condition( MyErrorCodes::Client );
		}
		else if ( ( error_value >= MyErrorCodes::Server ) && ( error_value < MyErrorCodes::Ipc ) )
		{
			return std::error_condition( MyErrorCodes::Server );
		}
		else if ( ( error_value >= MyErrorCodes::Ipc ) && ( error_value < MyErrorCodes::Other ) )
		{
			return std::error_condition( MyErrorCodes::Ipc );
		}
		return std::error_condition( MyErrorCodes::Other );
	}

	virtual bool equivalent( const std::error_code &code, int condition ) const noexcept  override
	{
		return *this == code.category() && static_cast<int>( default_error_condition( code.value() ).value() ) == condition;
	}

	virtual std::string message( int err_value ) const override
	{
		MyErrorCodes error_value = static_cast<MyErrorCodes>( err_value );
		switch( error_value )
		{
			case MyErrorCodes::ClientAbort:		return "Client abort";
			case MyErrorCodes::ServerBusy:		return "Server busy";
			case MyErrorCodes::IpcTimeout:		return "IPC timeout";
			case MyErrorCodes::IpcRead:			return "IPC read";
			case MyErrorCodes::IpcWrite:		return "IPC write";
			default: break;
		}
		if ( ( error_value >= MyErrorCodes::Client ) && ( error_value < MyErrorCodes::Server ) )
		{
			return "Client error";
		}
		else if ( ( error_value >= MyErrorCodes::Server ) && ( error_value < MyErrorCodes::Ipc ) )
		{
			return "Server error";
		}
		else if ( ( error_value >= MyErrorCodes::Ipc ) && ( error_value < MyErrorCodes::Other ) )
		{
			return "IPC error";
		}
		return "Unknown error";
	}
} MyErrorCategory;

std::error_condition make_error_condition( MyErrorCodes e )
{
	return std::error_condition( static_cast<int>( e ), MyErrorCategory );
}

int main()
{
	std::error_code e( MyErrorCodes::ClientAbort, MyErrorCategory );
	std::cout << e.message() << std::endl;

	e = std::error_code( MyErrorCodes::ServerBusy, MyErrorCategory );
	std::cout << e.message() << std::endl;

	e = std::error_code( MyErrorCodes::IpcRead, MyErrorCategory );
	std::cout << e.message() << std::endl;

	e = std::error_code( 222, MyErrorCategory );
	std::cout << e.message() << std::endl;

	e = std::error_code( 567, MyErrorCategory );
	std::cout << e.message() << std::endl;

	return 0;
}
