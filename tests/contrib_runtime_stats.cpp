#include "../contrib/runtime_stats.hpp"

#include "test_core.hpp"

int test()
{
	CHECK( getPeakRSS() >= 0 );
	CHECK( getCurrentRSS() >= 0 );
	CHECK( getCPUTime() >= 0 );
	
	return 0;
}

#include "../contrib/runtime_stats.cpp"
