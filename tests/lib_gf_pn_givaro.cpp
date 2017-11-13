

/***************************************************************************** \
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

#include "../lib/gf_elem_simple.hpp"
#include "../lib/gf_pn_givaro.hpp"

#include "test_core.hpp"

const std::size_t FIELDCHAR = 3;
const std::size_t EXTDEG = 2;

template<class givfield_t>
int test_field()
{
	typedef typename givfield_t::gfelm_t elem_t;

	static const auto & givfield = givfield_t::givaro_field;

	CHECK((givfield_t::gfchar == givfield.characteristic()));
	CHECK((givfield_t::extdeg == givfield.exponent()));
	CHECK((givfield_t::gfsize == givfield.size()));
	CHECK((givfield_t::_qm1   == givfield.size() - 1));
	CHECK((givfield.irreducible() == 17));
	CHECK((givfield.generator() == 3));

	CHECK((givfield.zech2padic(1) == 3));
	CHECK((givfield.zech2padic(2) == 4));
	CHECK((givfield.zech2padic(3) == 7));
	CHECK((givfield.zech2padic(4) == 2));
	CHECK((givfield.zech2padic(5) == 6));
	CHECK((givfield.zech2padic(6) == 8));
	CHECK((givfield.zech2padic(7) == 5));
	CHECK((givfield.zech2padic(8) == 1));

	elem_t gen = givfield.padic2zech( givfield.generator() );
	elem_t e = givfield.one;
	e *= gen; CHECK((e == (elem_t) 1));
	e *= gen; CHECK((e == (elem_t) 2));
	e *= gen; CHECK((e == (elem_t) 3));
	e *= gen; CHECK((e == (elem_t) 4));
	e *= gen; CHECK((e == (elem_t) 5));
	e *= gen; CHECK((e == (elem_t) 6));
	e *= gen; CHECK((e == (elem_t) 7));
	e *= gen; CHECK((e == (elem_t) 8));

	CHECK(( (elem_t) 1 + (elem_t) 3 == (elem_t) 8 ));
	CHECK(( (elem_t) 1 - (elem_t) 3 == (elem_t) 6 ));
	CHECK(( (elem_t) 1 * (elem_t) 3 == (elem_t) 4 ));
	CHECK(( (elem_t) 1 / (elem_t) 3 == (elem_t) 6 ));
	CHECK(( gb::mul((elem_t)1, (elem_t)3) == (elem_t) 4 ));
	CHECK(( gb::div((elem_t)1, (elem_t)3) == (elem_t) 6 ));
	CHECK(( gb::mul_nonzero((elem_t)1, (elem_t)3) == (elem_t) 4 ));
	CHECK(( gb::div_nonzero((elem_t)1, (elem_t)3) == (elem_t) 6 ));

	CHECK(( (elem_t) 1 == (elem_t) 1 ));
	CHECK(( (elem_t) 1 != (elem_t) 0 ));

	std::array<elem_t, 9> L = {{1, 2, 3, 4, 5, 6, 7, 8, 0}};
	std::array<elem_t, 9> R = {{0, 1, 2, 3, 4, 5, 6, 7, 8}};

	gb::mul_to(&L[0], (elem_t) 1, L.size()); //multiply with generator
	CHECK(( L[0] == (elem_t) 2));
	CHECK(( L[1] == (elem_t) 3));
	CHECK(( L[2] == (elem_t) 4));
	CHECK(( L[3] == (elem_t) 5));
	CHECK(( L[4] == (elem_t) 6));
	CHECK(( L[5] == (elem_t) 7));
	CHECK(( L[6] == (elem_t) 8));
	CHECK(( L[7] == (elem_t) 1));
	CHECK(( L[8] == (elem_t) 0));

	gb::add_to(&L[0], &R[0], L.size());
	CHECK(( L[0] == (elem_t) 2));
	CHECK(( L[1] == (elem_t) 8));
	CHECK(( L[2] == (elem_t) 1));
	CHECK(( L[3] == (elem_t) 2));
	CHECK(( L[4] == (elem_t) 3));
	CHECK(( L[5] == (elem_t) 4));
	CHECK(( L[6] == (elem_t) 5));
	CHECK(( L[7] == (elem_t) 6));
	CHECK(( L[8] == (elem_t) 8));

	gb::substract_to(&L[0], &R[0], L.size());
	CHECK(( L[0] == (elem_t) 2));
	CHECK(( L[1] == (elem_t) 3));
	CHECK(( L[2] == (elem_t) 4));
	CHECK(( L[3] == (elem_t) 5));
	CHECK(( L[4] == (elem_t) 6));
	CHECK(( L[5] == (elem_t) 7));
	CHECK(( L[6] == (elem_t) 8));
	CHECK(( L[7] == (elem_t) 1));
	CHECK(( L[8] == (elem_t) 0));

	gb::add_to(&L[0], (elem_t) 1, &R[0], L.size());
	CHECK(( L[0] == (elem_t) 2));
	CHECK(( L[1] == (elem_t) 4));
	CHECK(( L[2] == (elem_t) 5));
	CHECK(( L[3] == (elem_t) 6));
	CHECK(( L[4] == (elem_t) 7));
	CHECK(( L[5] == (elem_t) 8));
	CHECK(( L[6] == (elem_t) 1));
	CHECK(( L[7] == (elem_t) 2));
	CHECK(( L[8] == (elem_t) 1));

	return 0;
}

int test()
{
	typedef gb::giv_modpoly<2, 2, 1> modpoly; //x^2 + 2x + 1
	typedef gb::giv_genpoly<0, 1> genpoly; //x
	typedef gb::gf_pn_givaro<FIELDCHAR, EXTDEG, modpoly, genpoly> givfield_t;

	test_field<givfield_t>();

	return 0;
}
