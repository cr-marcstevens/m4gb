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

#ifndef M4GB_GF2N_HPP
#define M4GB_GF2N_HPP


#include "detail.hpp"
#include "logger.hpp"
#include "gf_elem_simple.hpp"

#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

namespace gb
{

	namespace detail
	{

		template<std::size_t N> struct gf_2n_poly     { };
		template<>              struct gf_2n_poly < 1 > { static const std::size_t poly = 2; };
		template<>              struct gf_2n_poly < 2 > { static const std::size_t poly = 7; };
		template<>              struct gf_2n_poly < 3 > { static const std::size_t poly = 11; };
		template<>              struct gf_2n_poly < 4 > { static const std::size_t poly = 19; };
		template<>              struct gf_2n_poly < 5 > { static const std::size_t poly = 37; };
		template<>              struct gf_2n_poly < 6 > { static const std::size_t poly = 67; };
		template<>              struct gf_2n_poly < 7 > { static const std::size_t poly = 131; };
		template<>              struct gf_2n_poly < 8 > { static const std::size_t poly = 285; };
		template<>              struct gf_2n_poly < 9 > { static const std::size_t poly = 515; };
		template<>              struct gf_2n_poly < 10 > { static const std::size_t poly = 1033; };
		template<>              struct gf_2n_poly < 11 > { static const std::size_t poly = 2053; };
		template<>              struct gf_2n_poly < 12 > { static const std::size_t poly = 4105; };
		template<>              struct gf_2n_poly < 13 > { static const std::size_t poly = 8219; };
		template<>              struct gf_2n_poly < 14 > { static const std::size_t poly = 16417; };
		template<>              struct gf_2n_poly < 15 > { static const std::size_t poly = 33025; };
		template<>              struct gf_2n_poly < 16 > { static const std::size_t poly = 65607; };

		template<std::size_t gfsize, std::size_t gfpoly, typename elem_t = typename gf_elem_types<gfsize>::elem_t>
		elem_t mul_gf2n(elem_t x, elem_t y)
		{
			if (x >= gfsize || y >= gfsize)
				throw std::runtime_error("mul_gf2n(x,y): x or y out of range");
			elem_t r = 0;
			typename gf_elem_types<gfsize>::dblelem_t yy = y;
			while (x != 0)
			{
				if (x & 1)
					r ^= (elem_t)(yy);
				x >>= 1;
				yy <<= 1;
				if ((yy ^ gfpoly) < yy)
					yy ^= gfpoly;
			}
			return r;
		}

		template<std::size_t gfsize, std::size_t gfpoly, typename elem_t = typename gf_elem_types<gfsize>::elem_t>
		elem_t pow_gf2n(elem_t base, size_t ext)
		{
			if (base >= gfsize)
				throw std::runtime_error("pow_gf2n(x,y): base out of range");
			if (ext == 0)
				return 1;

			elem_t r = base;

			int b = 0;
			while ((ext >> b) != 0)
				++b;

			for (b -= 2; b >= 0; --b)
			{
				r = mul_gf2n<gfsize, gfpoly>(r, r);
				if ((ext >> b) & 1)
					r = mul_gf2n<gfsize, gfpoly>(r, base);
			}
			return r;
		}
	} // namespace detail

	// generic template for finite fields of type GF(2^N)
	template<std::size_t N, std::size_t Poly = detail::gf_2n_poly<N>::poly>
	class gf_2n_simple
	{
	public:
		static const std::size_t gfchar = 2;
		static const std::size_t gfsize = std::size_t(1) << N;
		static const std::size_t fieldchar = 2;
		static const std::size_t fieldsize = std::size_t(1) << N;

		typedef typename gf_elem_types<gfsize>::elem_t elem_t;
		typedef typename gf_elem_types<gfsize>::dblelem_t dblelem_t;
		typedef typename gf_elem_types<gfsize>::multelem_t multelem_t;
		typedef typename gf_elem_types<gfsize>::nonchar_elem_t nonchar_elem_t;

		typedef gfelm< gf_2n_simple<N,Poly> > gfelm_t;

		static const dblelem_t gfpoly = Poly;

		struct static_data_t {
			elem_t prim_elem;
			elem_t log_table[gfsize * 2];
			elem_t ilog_table[gfsize * 2];
#ifdef GF_USE_MUL_TABLE
			elem_t mul_table[gfsize][gfsize];
#endif

			static_data_t()
			{
				// find primitive element
				std::vector<std::size_t> factors = detail::factor_int(gfsize - 1);
				std::sort(factors.begin(), factors.end());
				factors.erase(std::unique(factors.begin(), factors.end()), factors.end());
				for (prim_elem = 2;; ++prim_elem)
				{
					bool ok = true;
					for (auto it = factors.begin(); it != factors.end(); ++it)
					{
						std::size_t ordit = detail::pow_gf2n<gfsize, gfpoly>(prim_elem, (elem_t)((gfsize - 1) / *it));
						if (ordit == 1)
						{
							ok = false;
							break;
						}
					}
					if (ok)
						break;
				}

				// found primitive element, generate all elements
				std::vector<elem_t> elems;
				elems.push_back(0);
				for (std::size_t i = 1; i <= gfsize - 1; ++i)
					elems.push_back(detail::pow_gf2n<gfsize, gfpoly>(prim_elem, i));
				std::sort(elems.begin(), elems.end());
				elems.erase(std::unique(elems.begin(), elems.end()), elems.end());
				if (elems.size() != gfsize)
					throw std::runtime_error("gf2n::gf2n(): internal error: primitive element not primitive");

				// generate tables
				static_data.log_table[0] = 0;
				static_data.ilog_table[0] = 1;
				for (std::size_t i = 0; i < gfsize - 1; ++i)
				{
					elem_t y = detail::pow_gf2n<gfsize, gfpoly>(prim_elem, i);
					static_data.log_table[y] = i;
					static_data.ilog_table[i] = y;
				}
				for (unsigned i = gfsize - 1; i < 2 * gfsize; ++i)
					static_data.ilog_table[i] = static_data.ilog_table[i - (gfsize - 1)];
				for (unsigned i = gfsize; i < 2 * gfsize; ++i)
					static_data.log_table[i] = static_data.log_table[i - gfsize];

#ifdef GF_USE_MUL_TABLE
				for (unsigned i = 0; i < gfsize; ++i)
					for (unsigned j = 0; j < gfsize; ++j)
						mul_table[i][j] = (i == 0 || j == 0) ? 0 : static_data.ilog_table[(dblelem_t)(static_data.log_table[i]) + (dblelem_t)(static_data.log_table[j])];
#endif

				get_logger()("gf2n",GF_LOGLEVEL) << "gf2n<" << N << "> initialized." << std::endl;
			}
		};
		static static_data_t static_data;

		int test()
		{
			get_logger()("gf2n", GF_LOGLEVEL) << "Testing gf2n<" << N << ">...";
			for (unsigned i = 0; i < gfsize; ++i)
				for (unsigned j = 0; j < gfsize; ++j)
				{
					elem_t prod = mult(i, j);
					std::size_t prod2 = detail::mul_gf2n<gfsize, gfpoly>(i, j);
					if (prod2 != prod)
						get_logger().msg("gf2n", "gf2n::gf2n(): internal error: multiplication table error", lg_abort);
					if (i != 0 && j != div(prod, i))
						get_logger().msg("gf2n", "gf2n::gf2n(): internal error: division table error", lg_abort);
					if (j != 0 && i != div(prod, j))
						get_logger().msg("gf2n", "gf2n::gf2n(): internal error: division table error", lg_abort);

					elem_t sum = add(i, j);
					std::size_t sum2 = i ^ j;
					if (sum2 != sum)
						get_logger().msg("gf2n", "gf2n::gf2n(): internal error: add table error", lg_abort);
					if (j != sub(sum, i))
						get_logger().msg("gf2n", "gf2n::gf2n(): internal error: sub table error", lg_abort);
					if (i != sub(sum, j))
						get_logger().msg("gf2n", "gf2n::gf2n(): internal error: sub table error", lg_abort);
				}
			get_logger()(GF_LOGLEVEL) << std::endl;
			return 0;
		}

		inline static elem_t add(const elem_t l, const elem_t r)
		{
			return l ^ r;
		}
		inline static elem_t sub(const elem_t l, const elem_t r)
		{
			return l ^ r;
		}
		inline static elem_t mult(const elem_t l, const elem_t r)
		{
#ifdef GF_USE_MUL_TABLE
			return static_data.mul_table[l][r];
#else
			if (l&&r)
				return static_data.ilog_table[(dblelem_t)(static_data.log_table[l]) + (dblelem_t)(static_data.log_table[r])];
			return 0;
#endif
		}
		inline static elem_t mult_nonzero(const elem_t l, const elem_t r)
		{
#ifdef GF_USE_MUL_TABLE
			return static_data.mul_table[l][r];
#else
			return static_data.ilog_table[(dblelem_t)(static_data.log_table[l]) + (dblelem_t)(static_data.log_table[r])];
#endif
		}
		inline static elem_t div(const elem_t l, const elem_t r)
		{
			if (l&&r)
				return static_data.ilog_table[(dblelem_t)(gfsize - 1) - (dblelem_t)(static_data.log_table[r]) + (dblelem_t)(static_data.log_table[l])];
			return 0;
		}
		inline static elem_t div_nonzero(const elem_t l, const elem_t r)
		{
			return static_data.ilog_table[(dblelem_t)(gfsize - 1) - (dblelem_t)(static_data.log_table[r]) + (dblelem_t)(static_data.log_table[l])];
		}
		inline static elem_t negate(const elem_t l)
		{
			return l;
		}

		/* vectorizable operations */
		inline static void mul_to(elem_t* l, const elem_t c, int len)
		{
			if (c == 0)
			{
				for (int i = 0; i < len; ++i)
					l[i] = 0;
			}
			else if (c == 1)
			{
			}
			else
			{
#ifdef GF_USE_MUL_TABLE
				for (int i = 0; i < len; ++i)
					l[i] = static_data.mul_table[c][ l[i] ];
#else
				dblelem_t cc = static_data.log_table[c];
				for (int i = 0; i < len; ++i)
					if (l[i] != 0)
						l[i] = static_data.ilog_table[(dblelem_t)(static_data.log_table[l[i]]) + cc];
#endif
			}
		}
		inline static void add_to(elem_t* l, const elem_t c, const elem_t* r, int len)
		{
			if (c == 0)
				return;
			if (c == 1)
			{
				add_to(l, r, len);
			}
			else
			{
#ifdef GF_USE_MUL_TABLE
				for (int i = 0; i < len; ++i)
					l[i] ^= static_data.mul_table[c][ r[i] ];
#else
				dblelem_t cc = static_data.log_table[c];
				for (int i = 0; i < len; ++i)
					if (r[i] != 0)
						l[i] ^= static_data.ilog_table[(dblelem_t)(static_data.log_table[r[i]]) + cc];
#endif
			}
		}
		inline static void add_to(elem_t* l, const elem_t* r, int len)
		{
			for (int i = 0; i < len; ++i)
				l[i] ^= r[i];
		}
		inline static void substract_to(elem_t* l, const elem_t* r, int len)
		{
			for (int i = 0; i < len; ++i)
				l[i] ^= r[i];
		}
	};

	template<std::size_t N, std::size_t Poly>
	typename gf_2n_simple<N, Poly>::static_data_t gf_2n_simple<N, Poly>::static_data;

} // namespace gb

#endif // M4GB_GF_HPP
