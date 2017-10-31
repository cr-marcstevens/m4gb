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

#ifndef M4GB_SOLVER_CONFIG_HPP
#define M4GB_SOLVER_CONFIG_HPP

#include "../lib/config.h"

#define PRINT_PROCESS_STATISTICS

#ifndef MAXVARS
#define MAXVARS   20
#endif

#ifndef FIELDSIZE
#define FIELDSIZE 31
#endif

#ifndef INT_MONOMIAL_SIZE
#define INT_MONOMIAL_SIZE 8
#endif

#if FIELDSIZE <= 256
#define GF_USE_MUL_TABLE
#endif

#if FIELDSIZE <= 64
#define GF_USE_ADD_MUL_TABLE
#endif

#if FIELDSIZE == 2
#define MONOMIAL_ALLOW_FIELDEQUATIONS
#define GB_ADD_FIELDEQUATIONS
#define MONOMIAL_NO_WRAPCHECK
#endif

#if FIELDSIZE >= 31
#define MONOMIAL_NO_WRAPCHECK
#endif

#define MQ_NO_TRY_CATCH

#include "../lib/gf_p_simple.hpp"
#include "../lib/gf_2n_simple.hpp"
#include "../lib/monomial_degrevlex.hpp"
#include "../lib/polynomial_simple.hpp"
#include "../lib/polynomial_int.hpp"

namespace gb
{
	namespace detail 
	{
		template<std::size_t fieldsize, std::size_t parity>
		struct getfield
		{
			static_assert( is_prime<fieldsize>::value, "An odd fieldsize must be a prime");
			typedef ::gb::gf_p_simple<fieldsize> type;
		};

		template<std::size_t fieldsize>
		struct getfield<fieldsize,0>
		{
			static const std::size_t extension = nrbits_t<fieldsize>::value - 1;
			static_assert( fieldsize == std::size_t(1) << extension, "An even fieldsize must be a power of 2" );
			typedef ::gb::gf_2n_simple<extension> type;
		};

		template<>
		struct getfield<2,0>
		{
			typedef ::gb::gf_2n_simple<1, 2> type;
		};
	}

	template<std::size_t fieldsize>
	struct getfield
	{
		typedef typename detail::getfield<fieldsize,fieldsize&1>::type type; 
	};

	typedef getfield<FIELDSIZE>::type myfield_t;
	typedef polynomial_simple_t<monomial_degrevlex_traits<MAXVARS, FIELDSIZE>, myfield_t> mypolynomial_t;
}

#endif