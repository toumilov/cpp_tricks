
#include <utility>

template <typename Lambda>
class ScopeExit
{
public:
	ScopeExit( const Lambda &f ) :
		on_exit_( std::move( f ) ),
		valid_( true )
	{}

	~ScopeExit() noexcept
	{
		if ( valid_ )
		{
			on_exit_();
		}
	}

	ScopeExit( const ScopeExit& ) = delete;
	ScopeExit& operator=( const ScopeExit& ) = delete;

	ScopeExit( ScopeExit &&rhs ) :
		on_exit_( std::move( rhs.on_exit_ ) ),
		valid_( rhs.valid_ )
	{
		rhs.valid_ = false;
	}

	ScopeExit& operator=( ScopeExit &&rhs )
	{
		on_exit_ = rhs.on_exit_;
		valid_ = rhs.valid_;
		rhs.valid_ = false;
		return *this;
	}

	void release() const
	{
		valid_ = false;
	}

private:
	Lambda on_exit_;
	mutable bool valid_;
};

template <typename Lambda>
ScopeExit<typename std::decay<Lambda>::type> on_scope_exit( Lambda &&f )
{
	return std::forward<Lambda>( f );
}
