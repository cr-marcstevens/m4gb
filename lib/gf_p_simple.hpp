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

#ifndef M4GB_GF_P_SIMPLE_HPP
#define M4GB_GF_P_SIMPLE_HPP

#include "detail.hpp"
#include "logger.hpp"
#include "gf_elem_simple.hpp"

#include <algorithm>

namespace gb
{
	
	namespace detail
	{

		template<std::size_t P, typename elem_t = typename gf_elem_types<P>::elem_t>
		elem_t pow_int_mod(elem_t base, elem_t ext)
		{
			if (ext == 0)
				return 1;
			base %= P;
			typename gf_elem_types<P>::multelem_t ret = base;
			int b = 0;
			while ((ext >> b) != 0)
				++b;
			for (b -= 2; b >= 0; --b)
			{
				ret *= ret;
				ret %= P;
				if ((ext >> b) & 1)
				{
					ret *= base;
					ret %= P;
				}
			}
			return (elem_t)(ret);
		}

		template<std::size_t P>
		struct gfp_mul_table 
		{
			static const std::size_t width = std::size_t(1) << nrbits_t<P>::value;
			typedef typename gf_elem_types<P>::elem_t elem_t;
			elem_t mul[width][width];
			
			gfp_mul_table()
			{
				for (std::size_t i = 0; i < P; ++i)
					for (std::size_t j = 0; j < P; ++j)
						mul[i][j] = (i*j) % P;
			}
		};

		template<std::size_t P>
		struct gfp_addmul_table 
		{
			static const std::size_t width = std::size_t(1) << nrbits_t<P>::value;
			typedef typename gf_elem_types<P>::elem_t elem_t;
			elem_t addmul[width][width][width];
			
			gfp_addmul_table()
			{
				for (std::size_t i = 0; i < P; ++i)
					for (std::size_t j = 0; j < P; ++j)
						for (std::size_t k = 0; k < P; ++k)
							addmul[i][j][k] = ((i*j)+k) % P;
			}
		};

	} // namespace detail
	
	// generic template for prime finite field
	template<std::size_t P>
	class gf_p_simple
	{
	public:
		static const std::size_t gfchar = P;
		static const std::size_t gfsize = P;
		static const std::size_t fieldchar = P;
		static const std::size_t fieldsize = P;

		typedef typename gf_elem_types<gfsize>::elem_t elem_t;
		typedef typename gf_elem_types<gfsize>::dblelem_t dblelem_t;
		typedef typename gf_elem_types<gfsize>::multelem_t multelem_t;
		typedef typename gf_elem_types<gfsize>::nonchar_elem_t nonchar_elem_t;

		struct static_data_t
		{
			elem_t prim_elem;
			elem_t log_table[2 * gfsize];
			elem_t ilog_table[2 * gfsize];
			elem_t trunc_table[2 * gfsize];
#ifdef GF_USE_MUL_TABLE
			detail::gfp_mul_table<P> mul_table;
#endif
#ifdef GF_USE_ADD_MUL_TABLE
			detail::gfp_addmul_table<P> addmul_table;
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
						elem_t ordit = detail::pow_int_mod<gfchar>(prim_elem, (elem_t)((gfsize - 1) / *it));
						if (ordit == 1)
						{
							ok = false;
							break;
						}
					}
					if (ok)
						break;
				}

				// found primitive element, test by generating all elements
				std::vector<elem_t> elems;
				elems.push_back(0);
				for (elem_t i = 1; i <= gfsize - 1; ++i)
					elems.push_back(detail::pow_int_mod<gfchar>(prim_elem, i));
				std::sort(elems.begin(), elems.end());
				elems.erase(std::unique(elems.begin(), elems.end()), elems.end());
				if (elems.size() != gfsize)
					throw std::runtime_error("gf_p_simple::gf_p_simple(): internal error: primitive element not primitive");

				// generate tables
				static_data.log_table[0] = 0;
				static_data.ilog_table[0] = 1;
				for (elem_t i = 0; i < gfsize - 1; ++i)
				{
					elem_t y = detail::pow_int_mod<gfsize>(prim_elem, i);
					static_data.log_table[y] = i;
					static_data.ilog_table[i] = y;
				}
				for (unsigned i = gfsize - 1; i < 2 * gfsize; ++i)
					static_data.ilog_table[i] = static_data.ilog_table[i - (gfsize - 1)];
				for (unsigned i = gfsize; i < 2 * gfsize; ++i)
					static_data.log_table[i] = static_data.log_table[i - gfsize];

				for (dblelem_t i = 0; i < 2 * gfsize; ++i)
					static_data.trunc_table[i] = (elem_t)(i%gfsize);

				get_logger()("gf_p_simple", GF_LOGLEVEL) << "gf_p_simple<" << P << "> initialized." << std::endl;
			}
		};
		static static_data_t static_data;

		int test()
		{
			// test tables
			get_logger()("gf_p_simple", GF_LOGLEVEL) << "Testing gf_p_simple<" << P << ">...";
			for (unsigned i = 0; i < gfsize; ++i)
				for (unsigned j = 0; j < gfsize; ++j)
				{
					elem_t prod = mult(i, j);
					elem_t prod2 = (elem_t)((multelem_t(i)*multelem_t(j)) % multelem_t(gfchar));
					if (prod2 != prod)
						get_logger().msg("gf_p_simple", "gf_p_simple::gf_p_simple(): internal error: multiplication table error", lg_abort);
					if (i != 0 && j != div(prod, i))
						get_logger().msg("gf_p_simple", "gf_p_simple::gf_p_simple(): internal error: division table error", lg_abort);
					if (j != 0 && i != div(prod, j))
						get_logger().msg("gf_p_simple", "gf_p_simple::gf_p_simple(): internal error: division table error", lg_abort);

					elem_t sum = add(i, j);
					elem_t sum2 = (elem_t)((multelem_t(i) + multelem_t(j)) % multelem_t(gfchar));
					if (sum2 != sum)
						get_logger().msg("gf_p_simple", "gf_p_simple::gf_p_simple(): internal error: add table error", lg_abort);
					if (j != sub(sum, i))
						get_logger().msg("gf_p_simple", "gf_p_simple::gf_p_simple(): internal error: sub table error", lg_abort);
					if (i != sub(sum, j))
						get_logger().msg("gf_p_simple", "gf_p_simple::gf_p_simple(): internal error: sub table error", lg_abort);
				}
			get_logger()(GF_LOGLEVEL) << std::endl;
			return 0;
		}
		inline static elem_t add(const elem_t l, const elem_t r)
		{
			return static_data.trunc_table[(dblelem_t)(l)+(dblelem_t)(r)];
		}
		inline static elem_t sub(const elem_t l, const elem_t r)
		{
			return static_data.trunc_table[(dblelem_t)(gfsize)-(dblelem_t)(r)+(dblelem_t)(l)];
		}
		inline static elem_t mult(const elem_t l, const elem_t r)
		{
#ifdef GF_USE_MUL_TABLE
			return static_data.mul_table.mul[l][r];
#else
			if (l&&r)
				return static_data.ilog_table[(dblelem_t)(static_data.log_table[l]) + (dblelem_t)(static_data.log_table[r])];
			return 0;
#endif
		}
		inline static elem_t mult_nonzero(const elem_t l, const elem_t r)
		{
#ifdef GF_USE_MUL_TABLE
			return static_data.mul_table.mul[l][r];
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
			return (l == 0) ? (0) : (gfsize - l);
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
#ifdef GF_USE_MUL_TABLE
			else
			{
				for (int i = 0; i < len; ++i)
					l[i] = static_data.mul_table.mul[c][ l[i] ];
			}
#else
			else if (c == gfsize - 1)
			{
				for (int i = 0; i < len; ++i)
					l[i] = (l[i] == 0) ? (0) : (gfsize - l[i]);
			}
			else
			{
				dblelem_t cc = static_data.log_table[c];
				for (int i = 0; i < len; ++i)
					if (l[i] != 0)
						l[i] = static_data.ilog_table[(dblelem_t)(static_data.log_table[l[i]]) + cc];

			}
#endif
		}
		inline static void add_to(elem_t* l, const elem_t c, const elem_t* r, int len)
		{
			if (c == 0)
				return;
			if (c == 1)
			{
				add_to(l, r, len);
			}
			else if (c == gfsize - 1)
			{
				substract_to(l, r, len);
			}
#ifdef GF_USE_ADD_MUL_TABLE
			else
			{
				for (int i = 0; i < len; ++i)
					l[i] = static_data.addmul_table.addmul[c][ r[i] ][ l[i] ];
			}
#else
#ifdef GF_USE_MUL_TABLE
			else
			{
				for (int i = 0; i < len; ++i)
					l[i] = add(l[i], static_data.mul_table.mul[c][ r[i] ]);
			}
#else
			else
			{
				dblelem_t cc = static_data.log_table[c];
				for (int i = 0; i < len; ++i)
					if (r[i] != 0)
						l[i] = add(l[i], static_data.ilog_table[(dblelem_t)(static_data.log_table[r[i]]) + cc]);
			}
#endif // GF_USE_MUL_TABLE
#endif // GF_USE_ADD_MUL_TABLE
		}
		inline static void add_to(elem_t* l, const elem_t* r, int len)
		{
			for (int i = 0; i < len; ++i)
				l[i] = add(l[i], r[i]);
		}
		inline static void substract_to(elem_t* l, const elem_t* r, int len)
		{
			for (int i = 0; i < len; ++i)
				l[i] = sub(l[i], r[i]);
		}
	};

	template<std::size_t P>
	typename gf_p_simple<P>::static_data_t gf_p_simple<P>::static_data;

} // namespace gb

#endif // M4GB_GF_P_SIMPLE_HPP
