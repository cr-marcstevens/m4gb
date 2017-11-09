
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

template<class Field>
int test_field(const Field & Fq)
{
	std::cout << "exponent = " << Fq.exponent() << std::endl;
	std::cout << "size     = " << Fq.size() << std::endl;
	std::cout << "irred    = " << Fq.irreducible() << std::endl;


	/* test operation in gf::gfelm */
	typedef gb::gfelm<gb::gf_pn_givaro<FIELDCHAR, EXTDEG, Field>> elem_t;
	elem_t gen, e0, e1;


	gen = Fq.generator();

	e0 = 2;
	e1 = 7;

	std::cout << "generator = " << gen << std::endl;
	std::cout << e0 << " + " << e1 << " = " << e0 + e1 << std::endl;
	std::cout << e0 << " - " << e1 << " = " << e0 - e1 << std::endl;
	std::cout << e0 << " * " << e1 << " = " << e0 * e1 << std::endl;
	std::cout << e0 << " / " << e1 << " = " << e0 / e1 << std::endl;
	std::cout << e0 << " mul " << e1 << " = " << gb::mul(e0, e1) << std::endl;
	std::cout << e0 << " div " << e1 << " = " << gb::div(e0, e1) << std::endl;
	std::cout << e0 << " mul_nonzero " << e1 << " = " << gb::mul_nonzero(e0, e1) << std::endl;
	std::cout << e0 << " div_nonzero " << e1 << " = " << gb::div_nonzero(e0, e1) << std::endl;

	std::cout << e0 << " == " << e1 << " : " << (e0 == e1) << std::endl;
	std::cout << e0 << " != " << e1 << " : " << (e0 != e1) << std::endl;
	std::cout << e0 << " < " << e1 << "  : " << (e0 < e1) << std::endl;
	std::cout << e0 << " <= " << e1 << " : " << (e0 <= e1) << std::endl;
	std::cout << e0 << " > " << e1 << "  : " << (e0 > e1) << std::endl;
	std::cout << e0 << " >= " << e1 << " : " << (e0 >= e1) << std::endl;

	std::cout << e0 << " == " << 7 << " : " << (e0 == 7) << std::endl;
	std::cout << e0 << " != " << 7 << " : " << (e0 != 7) << std::endl;
	std::cout << e0 << " < " << 7 << "	: " << (e0 < 7) << std::endl;
	std::cout << e0 << " <= " << 7 << " : " << (e0 <= 7) << std::endl;
	std::cout << e0 << " > " << 7 << "	: " << (e0 > 7) << std::endl;
	std::cout << e0 << " >= " << 7 << " : " << (e0 >= 7) << std::endl;

	std::array<elem_t, 4> L = {{1, 1, 1, 1}};
	std::array<elem_t, 4> R = {{4, 3, 2, 1}};

	gb::mul_to(&L[0], e0, 4);
	std::cout << L[0] << " " << L[1] << " " << L[2] << " " << L[3] << std::endl;

	gb::add_to(&L[0], e0, &R[0], 4);
	std::cout << L[0] << " " << L[1] << " " << L[2] << " " << L[3] << std::endl;

	gb::add_to(&L[0], &R[0], 4);
	std::cout << L[0] << " " << L[1] << " " << L[2] << " " << L[3] << std::endl;

	gb::substract_to(&L[0], &R[0], 4);
	std::cout << L[0] << " " << L[1] << " " << L[2] << " " << L[3] << std::endl;

	return 0;
}

int test()
{
	const std::array<int32_t, 3> F9modulus = {{2, 2, 1}};// x^2 + 2x + 2
	const std::array<int32_t, 3> F9generator = {{0, 1, 0}};
	Givaro::GFqDom<int32_t> GFDom(FIELDCHAR, EXTDEG,
		F9modulus, F9generator);
	test_field(GFDom);

	return 0;
}
