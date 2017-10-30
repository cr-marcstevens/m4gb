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

#include "../lib/parser.hpp"
#include "../lib/polynomial_simple.hpp"
#include "../lib/monomial_degrevlex.hpp"
#include "../lib/gf_p_simple.hpp"
#include "../lib/solver_base.hpp"
#include "test_core.hpp"

int test()
{
	typedef gb::gf_p_simple<521> gf_t;
	typedef gb::monomial_degrevlex_traits_uint64<20,20> traits_t;
	typedef gb::polynomial_simple_t<traits_t, gf_t> polynomial_t;
	typedef gb::parser_t<polynomial_t> parser_t;
	typedef gb::dummy_solver_t<polynomial_t> solver_t;

	solver_t solver;
	parser_t parser;
	polynomial_t p = parser.parse_string("1 * x1 * x4+5 * x4 * y3");
	std::cout << p << std::endl;
	return 0;
}

