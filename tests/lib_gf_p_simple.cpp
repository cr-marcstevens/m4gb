#include "../lib/gf_p_simple.hpp"

#include "test_core.hpp"

int test()
{
	gb::gf_p_simple<3> gf_3;
	CHECK( gf_3.test() == 0);

	gb::gf_p_simple<31> gf_31;
	CHECK( gf_31.test() == 0);

	gb::gf_p_simple<521> gf_521;
	CHECK( gf_521.test() == 0);
	
	return 0;
}

