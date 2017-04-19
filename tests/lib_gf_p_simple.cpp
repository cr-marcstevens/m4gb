#include "../lib/gf_p_simple.hpp"

#include "test_core.hpp"

template<std::size_t P>
int test_P()
{
	gb::gf_p_simple<P> gf_P;
	CHECK( gf_P.test() == 0);

	typedef typename gb::gf_p_simple<P>::gfelm_t elm_t;

	std::size_t i = 1;
	elm_t x = 1;
	while (x != 0)
	{
		++i;
		x+=1;
	}
	CHECK( i == P );

	x = 1;
	for (i = 0; i < P; ++i)
		for (std::size_t j = 1; j < P; ++j)
		{
			elm_t y = i, z = j, d = y / z, m = y * z;
			CHECK( (y==z) == (i==j) );
			CHECK( (y!=z) == (i!=j) );
			CHECK( y * z == ((i*j)%P) );
			CHECK( y + z == ((i+j)%P) );
			CHECK( y - z == ((P+i-j)%P) );
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
	CHECK( test_P<3>() == 0 );
	CHECK( test_P<31>() == 0 );
	CHECK( test_P<521>() == 0 );

	return 0;
}

