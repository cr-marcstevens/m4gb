#include "../lib/gf_2n_simple.hpp"

#include "test_core.hpp"

template<std::size_t N>
int test_2N()
{
	gb::gf_2n_simple<N> gf_2n;
	CHECK( gf_2n.test() == 0);

	typedef typename gb::gf_2n_simple<N>::gfelm_t elm_t;

	std::size_t i = 1;
	elm_t x = 1;
	while (x != 0)
	{
		++i;
		x+=1;
	}
	CHECK( i == 2 );

	x = 1;
	for (i = 0; i < (1<<N); ++i)
		for (std::size_t j = 1; j < (1<<N); ++j)
		{
			elm_t y = i, z = j, d = y / z, m = y * z;
			CHECK( y == i );
			CHECK( z == j );
			CHECK( (y==z) == (i==j) );
			CHECK( (y!=z) == (i!=j) );
			CHECK( d * z == y );
			CHECK( m / z == y );
			CHECK( z + y - z == y);
			CHECK( (y*z)/z == y);
			CHECK( (y/z)*z == y);
			CHECK( (z/z) == 1);
		}

	return 0;
}

int test()
{
	CHECK( test_2N<2>() == 0 );
	CHECK( test_2N<3>() == 0 );
	CHECK( test_2N<4>() == 0 );
	CHECK( test_2N<5>() == 0 );
	CHECK( test_2N<6>() == 0 );
	CHECK( test_2N<7>() == 0 );
	CHECK( test_2N<8>() == 0 );
	CHECK( test_2N<9>() == 0 );

	return 0;
}

