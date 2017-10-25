#include "../contrib/runtime_stats.hpp"

#include "test_core.hpp"

#include <vector>

int test()
{
	CHECK( getPeakRSS() >= 0 );
	CHECK( getCurrentRSS() >= 0 );
	CHECK( getCPUTime() >= 0 );

	// we assume our test binary is smaller than 10MiB
	CHECK( getPeakRSS() <= 10*(1<<20) );
	CHECK( getCurrentRSS() <= 10*(1<<20) );

	{
		// we initialize a buffer larger than 10MiB
		std::vector<char> large_data(10*(1<<20));
		for (unsigned i = 0; i < large_data.size(); ++i)
			large_data[i] = (char)(i);

		// now both measures need to be larger than 10MiB
		CHECK( getPeakRSS() >= 10*(1<<20) );
		CHECK( getCurrentRSS() >= 10*(1<<20) );
	}

	// buffer is deallocated, so peak should be larger than 10MiB, currentRSS smaller.
	CHECK( getPeakRSS() >= 10*(1<<20) );
	CHECK( getCurrentRSS() <= 10*(1<<20) );

	return 0;
}

#include "../contrib/runtime_stats.cpp"
