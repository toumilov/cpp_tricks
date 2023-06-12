#include <cstdio>
#include <cassert>
#include <cmath>
#include <type_traits>

template <typename T>
class CumulativeAverage
{
	static_assert(std::is_arithmetic<T>::value, "Type must be numeric");
public:
	CumulativeAverage() :
		n_( 0 ),
		ca_( 0.0 )
	{}

	T get() const
	{
		return (T)ca_;
	}

	CumulativeAverage& put( const T value )
	{
		n_++;
		ca_ = ca_ + ( ( (double)value - ca_ ) / n_ );
		return *this;
	}

protected:
	size_t n_;	// Sample count
	double ca_;	// Current CA
};

void test_int()
{
	CumulativeAverage<int> avg;
	assert( avg.get() == 0 && "unexpected value" );
	assert( avg.put( 2 ).get() == 2 && "unexpected value" );
	assert( avg.put( 6 ).get() == 4 && "unexpected value" );
	assert( avg.put( 10 ).get() == 6 && "unexpected value" );
	assert( avg.put( -1 ).get() == 4 && "unexpected value" );
	assert( avg.put( 2 ).get() == 3 && "unexpected value" );
	assert( avg.put( 14 ).get() == 5 && "unexpected value" );
	printf( "%s OK\n", __func__ );
}

void test_uint()
{
	CumulativeAverage<unsigned> avg;
	assert( avg.get() == 0 && "unexpected value" );
	assert( avg.put( 2 ).get() == 2 && "unexpected value" );
	assert( avg.put( 6 ).get() == 4 && "unexpected value" );
	assert( avg.put( 10 ).get() == 6 && "unexpected value" );
	assert( avg.put( 1 ).get() == 4 && "unexpected value" );
	assert( avg.put( 2 ).get() == 4 && "unexpected value" );
	assert( avg.put( 14 ).get() == 5 && "unexpected value" );
	printf( "%s OK\n", __func__ );
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
	CumulativeAverage<double> avg;
	assert( doubles_eq( avg.get(), 0.0 ) && "unexpected value" );
	assert( doubles_eq( avg.put( 2.7 ).get(), 2.7 ) && "unexpected value" );
	assert( doubles_eq( avg.put( 6.1 ).get(), 4.4 ) && "unexpected value" );
	assert( doubles_eq( double_round( avg.put( 10.3 ).get() ), 6.37 ) && "unexpected value" );
	assert( doubles_eq( double_round( avg.put( 1.8 ).get() ), 5.23 ) && "unexpected value" );
	assert( doubles_eq( double_round( avg.put( 2.9 ).get() ), 4.76 ) && "unexpected value" );
	assert( doubles_eq( double_round( avg.put( 7.5 ).get() ), 5.22 ) && "unexpected value" );
	printf( "%s OK\n", __func__ );
}

int main()
{
	test_int();
	test_uint();
	test_float();
	return 0;
}
