#include <list>
#include <limits>
#include <memory>
#include <cstdio>
#include <cassert>
#include <cmath>

template <typename T>
class SimpleMovingAverage
{
	static_assert(std::is_arithmetic<T>::value, "Type must be numeric");
public:
	SimpleMovingAverage( size_t window_size ) :
		window_size_( window_size )
	{}

	T get() const
	{
		T ret = 0;
		for( const auto &i : items_ )
		{
			ret += i;
		}
		auto n = items_.size();
		return ret / ( ( n && n < window_size_ ) ? n : window_size_ );
	}

	SimpleMovingAverage& put( const T value )
	{
		items_.push_back( value );
		if ( items_.size() > window_size_ )
		{
			items_.pop_front();
		}
		return *this;
	}

protected:
	size_t window_size_;
	std::list<T> items_;
};

template <typename T>
class SMA_Optimized : public SimpleMovingAverage<T>
{
	static_assert(std::is_arithmetic<T>::value, "Type must be numeric");
	double value_;
public:
	SMA_Optimized( size_t window_size ) :
		SimpleMovingAverage<T>( window_size ),
		value_( 0.0 )
	{}

	T get() const
	{
		return value_;
	}

	SMA_Optimized& put( const T value )
	{
		this->items_.push_back( value );
		if ( this->items_.size() <= this->window_size_ )
		{
			value_ = 0.0;
			for( const auto &i : this->items_ )
			{
				value_ += i;
			}
			value_ /= this->items_.size();
		} else {
			T last = this->items_.front();
			this->items_.pop_front();
			double diff = value - last;
			diff /= (double)this->window_size_;
			value_ = value_ + diff;
			printf("<%f:%d>\n", value_, (int)value_);
		}
		return *this;
	}
};

void test_int( bool optimized = false )
{
	std::unique_ptr<SimpleMovingAverage<int>> avg;
	if ( optimized )
	{
		avg.reset( new SimpleMovingAverage<int>( 3 ) );
	} else {
		avg.reset( new SMA_Optimized<int>( 3 ) );
	}
	assert( avg->get() == 0 && "unexpected value" );
	assert( avg->put( 2 ).get() == 2 && "unexpected value" );
	assert( avg->put( 6 ).get() == 4 && "unexpected value" );
	assert( avg->put( 10 ).get() == 6 && "unexpected value" );
	assert( avg->put( -1 ).get() == 5 && "unexpected value" );
	assert( avg->put( 2 ).get() == 3 && "unexpected value" );
	assert( avg->put( 14 ).get() == 5 && "unexpected value" );
	printf( "%s%s OK\n", __func__, optimized ? " [optimized]" : "" );
}

void test_uint( bool optimized = false )
{
	std::unique_ptr<SimpleMovingAverage<unsigned>> avg;
	if ( optimized )
	{
		avg.reset( new SimpleMovingAverage<unsigned>( 3 ) );
	} else {
		avg.reset( new SMA_Optimized<unsigned>( 3 ) );
	}
	assert( avg->get() == 0 && "unexpected value" );
	assert( avg->put( 2 ).get() == 2 && "unexpected value" );
	assert( avg->put( 6 ).get() == 4 && "unexpected value" );
	assert( avg->put( 10 ).get() == 6 && "unexpected value" );
	assert( avg->put( 1 ).get() == 5 && "unexpected value" );
	assert( avg->put( 2 ).get() == 4 && "unexpected value" );
	assert( avg->put( 14 ).get() == 5 && "unexpected value" );
	printf( "%s%s OK\n", __func__, optimized ? " [optimized]" : "" );
}

void test_float( bool optimized = false )
{
	auto doubles_eq = []( double a, double b ) -> bool {
		return fabs( a - b ) < std::numeric_limits<double>::epsilon();
	};
	auto double_round = []( double v, unsigned decimal_places = 2 ) -> double {
		double m = std::pow( 10, decimal_places );
		return std::round( v * m ) / m;
	};
	std::unique_ptr<SimpleMovingAverage<double>> avg;
	if ( optimized )
	{
		avg.reset( new SimpleMovingAverage<double>( 3 ) );
	} else {
		avg.reset( new SMA_Optimized<double>( 3 ) );
	}
	assert( doubles_eq( avg->get(), 0.0 ) && "unexpected value" );
	assert( doubles_eq( avg->put( 2.7 ).get(), 2.7 ) && "unexpected value" );
	assert( doubles_eq( avg->put( 6.1 ).get(), 4.4 ) && "unexpected value" );
	assert( doubles_eq( double_round( avg->put( 10.3 ).get() ), 6.37 ) && "unexpected value" );
	assert( doubles_eq( double_round( avg->put( 1.8 ).get() ), 6.07 ) && "unexpected value" );
	assert( doubles_eq( double_round( avg->put( 2.9 ).get() ), 5.0 ) && "unexpected value" );
	assert( doubles_eq( double_round( avg->put( 7.5 ).get() ), 4.07 ) && "unexpected value" );
	printf( "%s%s OK\n", __func__, optimized ? " [optimized]" : "" );
}

int main()
{
	test_int();
	test_int( true );
	test_uint();
	test_uint( true );
	test_float();
	test_float( true );
	return 0;
}
