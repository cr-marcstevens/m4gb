/*****************************************************************************\
*                                                                             *
*   M4GB - an efficient Groebner-basis algorithm                              *
*   Copyright (C) 2017  Rusydi Makarim, Marc Stevens,                         *
*   Cryptology Group, Centrum Wiskunde & Informatica (CWI), The Netherlands   *
*                                                                             *
*   This file is part of M4GB.                                                *
*                                                                             *
*   M4GB is free software: you can redistribute it and/or modify              *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation, either version 3 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   M4GB is distributed in the hope that it will be useful,                   *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with M4GB.  If not, see <http://www.gnu.org/licenses/>.             *
*                                                                             *
\*****************************************************************************/

#include "../contrib/runtime_stats.hpp"

#include "test_core.hpp"

#include <vector>

int test()
{
	CHECK( getPeakRSS() >= 0 );
	CHECK( getCurrentRSS() >= 0 );
	CHECK( getCPUTime() >= 0 );

	// we assume our test binary is smaller than 10MiB
	std::cout << getPeakRSS() << " " << getCurrentRSS() << std::endl;
	CHECK( getPeakRSS() <= 10*(1<<20) );
	CHECK( getCurrentRSS() <= 10*(1<<20) );

	{
		// we initialize a buffer larger than 10MiB
		std::vector<char> large_data(10*(1<<20));
		for (unsigned i = 0; i < large_data.size(); ++i)
			large_data[i] = (char)(i);

		std::cout << getPeakRSS() << " " << getCurrentRSS() << std::endl;
		// now both measures need to be larger than 10MiB
		CHECK( getPeakRSS() >= 10*(1<<20) );
		CHECK( getCurrentRSS() >= 10*(1<<20) );
	}

	std::cout << getPeakRSS() << " " << getCurrentRSS() << std::endl;
	// buffer is deallocated, so peak should be larger than 10MiB, currentRSS smaller.
	CHECK( getPeakRSS() >= 10*(1<<20) );
	// when -fsanitize=address is used, memory is not actually freed
#ifndef __SANITIZE_ADDRESS__
/*	CHECK( getCurrentRSS() <= 10*(1<<20) ); fails on MacOSX*/
#endif
	return 0;
}

#include "../contrib/runtime_stats.cpp"
