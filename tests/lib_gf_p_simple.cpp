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
	CHECK( test_P<2>() == 0 );
	CHECK( test_P<3>() == 0 );
	CHECK( test_P<31>() == 0 );
	CHECK( test_P<521>() == 0 );

	return 0;
}

