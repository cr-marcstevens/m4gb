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

#ifndef M4GB_GF_PN_GIVARO_HPP
#define M4GB_GF_PN_GIVARO_HPP

#include <cmath>

#include "detail.hpp"
#include "logger.hpp"
#include "gf_elem_simple.hpp"

#include "../givaro/include/givaro/gfq.h"
#include "../givaro/include/givaro/gfqext.h"

namespace gb
{
	template<typename GivaroField>
	class givfield_exposer : GivaroField
	{
	public:
		typedef GivaroField givaro_field_t;
		typedef typename givaro_field_t::Element TT;
		typedef typename givaro_field_t::Residu_t UTT;

		static std::vector<TT> get_plus1(const GivaroField & B)
		{
			return B.*(&givfield_exposer::_plus1);
		}

		static std::vector<UTT> get_log2pol(const GivaroField & B)
		{
			return B.*(&givfield_exposer::_log2pol);
		}

		static std::vector<UTT> get_pol2log(const GivaroField & B)
		{
			return B.*(&givfield_exposer::_pol2log);
		}
	};

	template<std::size_t... Args>
	struct giv_modpoly
	{
		static const std::vector<std::size_t> data;
	};
	template<std::size_t... Args>
	const std::vector<std::size_t> giv_modpoly<Args...>::data = { Args... };

	template<std::size_t... Args>
	struct giv_genpoly
	{
		static const std::vector<std::size_t> data;
	};
	template<std::size_t... Args>
	const std::vector<std::size_t> giv_genpoly<Args...>::data = { Args... };


	template<std::size_t P, std::size_t N,
			 typename ModPoly, typename GenPoly,
			 typename GivaroField = Givaro::GFqDom<int32_t> >
	class gf_pn_givaro
	{
		static_assert(detail::is_prime<P>::value && P > 2, "template parameter P must be a prime number > 2");
		static_assert(N > 1, "template parameter N must be > 1");
	public:
		static const std::size_t gfchar = P;
		static const std::size_t extdeg = N;
		static const std::size_t gfsize = detail::pow<P, N>::value;
		static const std::size_t _qm1 = gfsize - 1;

		typedef GivaroField givaro_field_t;
		typedef typename givaro_field_t::Element  elem_t, nonchar_elem_t, TT;
		typedef typename givaro_field_t::Residu_t UTT;
		typedef typename std::vector<UTT>::size_type UT;
		typedef TT Rep;
		typedef gfelm< gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField> > gfelm_t;
		typedef std::vector<TT> _plus1_t;
		typedef std::vector<UTT> _log2pol_t, _pol2log_t;

		static givaro_field_t givaro_field;
		static _plus1_t _plus1;
		static _log2pol_t _log2pol;
		static _pol2log_t _pol2log;

		inline static elem_t add(const elem_t l, const elem_t r)
		{
			elem_t ret(l);
			_GIVARO_GFQ_ADD(ret, ret, r, _qm1, _plus1);
			return ret;
		}

		inline static elem_t sub(const elem_t l, const elem_t r)
		{
			elem_t ret(l);
			_GIVARO_GFQ_AUTOSUB(ret, r, givaro_field.mOne, _qm1, _plus1);
			return ret;
		}

		inline static elem_t mult(const elem_t l, const elem_t r)
		{
			elem_t ret(l);
			_GIVARO_GFQ_MUL(ret, ret, r, _qm1);
			return ret;
		}

		inline static elem_t mult_nonzero(const elem_t l, const elem_t r)
		{
			return mult(l, r);
		}

		inline static elem_t div(const elem_t l, const elem_t r)
		{
			elem_t ret(l);
			_GIVARO_GFQ_DIV(ret, ret, r, _qm1);
			return ret;
		}

		inline static elem_t div_nonzero(const elem_t l, const elem_t r)
		{
			return div(l, r);
		}

		inline static elem_t negate(const elem_t l)
		{
			elem_t ret(l);
			_GIVARO_GFQ_NEG(ret, ret, givaro_field.mOne, _qm1);
			return ret;
		}

		inline static void mul_to(elem_t* l, const elem_t c, int len)
		{
			for(int i = 0; i < len; ++i)
			{
				_GIVARO_GFQ_MUL(l[i], l[i], c, _qm1);
			}
		}

		inline static void add_to(elem_t* l, const elem_t c, const elem_t* r, int len)
		{
			Rep tmp;
			for(int i = 0; i < len; ++i)
			{
				tmp = l[i];
				_GIVARO_GFQ_MULADD(l[i], c, r[i], tmp, _qm1, _plus1);
			}
		}

		inline static void add_to(elem_t* l, const elem_t* r, int len)
		{
			for(int i = 0; i < len; ++i)
			{
				_GIVARO_GFQ_ADD(l[i], l[i], r[i], _qm1, _plus1);
			}
		}

		inline static void substract_to(elem_t* l, const elem_t* r, int len)
		{
			Rep tmp;
			for(int i = 0; i < len; ++i)
			{
				tmp = l[i];
				_GIVARO_GFQ_SUB(l[i], tmp, r[i], givaro_field.mOne, _qm1, _plus1);
			}
		}
	};

	template<std::size_t P, std::size_t N,
			 typename ModPoly, typename GenPoly,
			 typename GivaroField>
	typename gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::givaro_field_t
	gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::givaro_field =
		GivaroField(P, N, ModPoly::data, GenPoly::data);

	template<std::size_t P, std::size_t N,
			 typename ModPoly, typename GenPoly,
			 typename GivaroField>
	typename gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::_plus1_t
	gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::_plus1 =
		givfield_exposer<GivaroField>::get_plus1(gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::givaro_field);

	template<std::size_t P, std::size_t N,
			 typename ModPoly, typename GenPoly,
			 typename GivaroField>
	typename gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::_log2pol_t
	gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::_log2pol =
		givfield_exposer<GivaroField>::get_log2pol(gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::givaro_field);

	template<std::size_t P, std::size_t N,
			 typename ModPoly, typename GenPoly,
			 typename GivaroField>
	typename gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::_pol2log_t
	gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::_pol2log =
		givfield_exposer<GivaroField>::get_pol2log(gf_pn_givaro<P, N, ModPoly, GenPoly, GivaroField>::givaro_field);
}

#endif // M4GB_GF_PN_GIVARO_HPP
