#include "../lib/gf_2n_simple.hpp"

#include "test_core.hpp"

int test()
{
	gb::gf_2n_simple<2> gf_4;
	CHECK( gf_4.test() == 0);

	gb::gf_2n_simple<8> gf_256;
	CHECK( gf_256.test() == 0);

	gb::gf_2n_simple<12> gf_4096;
	CHECK( gf_4096.test() == 0);
	
	return 0;
}

