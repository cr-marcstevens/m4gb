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

#ifndef M4GB_GF_P_GIVARO_HPP
#define M4GB_GF_P_GIVARO_HPP

#include "../givaro/include/givaro/givinteger.h"
#include "../givaro/include/givaro/modular.h"

namespace gb
{
	template<std::size_t P, typename GivaroField>
	class gf_p_givaro
	{
	public:

		static const std::size_t gfchar = P;
		static const std::size_t gfsize = P;
		static const std::size_t fieldchar = P;
		static const std::size_t fieldsize = P;


		typedef GivaroField givaro_field_t;
		typedef typename givaro_field_t::Element Element;
		typedef typename givaro_field_t::Compute_t Compute_t;
		typedef Element elem_t, nonchar_elem_t;
		typedef gfelm< gf_p_givaro<P, GivaroField> > gfelm_t;

		static givaro_field_t givaro_field;

		inline static elem_t add(const elem_t l, const elem_t r)
		{
			elem_t ret;
			return givaro_field.add(ret, l, r);
		}

		inline static elem_t sub(const elem_t l, const elem_t r)
		{
			elem_t ret;
			return givaro_field.sub(ret, l, r);
		}

		inline static elem_t mult(const elem_t l, const elem_t r)
		{
			elem_t ret;
			return givaro_field.mul(ret, l, r);
		}

		inline static elem_t mult_nonzero(const elem_t l, const elem_t r)
		{
			return mult(l, r);
		}

		inline static elem_t div(const elem_t l, const elem_t r)
		{
			elem_t ret;
			return givaro_field.div(ret, l, r);
		}

		inline static elem_t div_nonzero(const elem_t l, const elem_t r)
		{
			return div(l, r);
		}

		inline static elem_t negate(const elem_t l)
		{
			elem_t ret;
			return givaro_field.neg(ret, l);
		}

		inline static void mul_to(elem_t* l, const elem_t c, int len)
		{
			for(int i = 0; i < len; ++i)
			{
				givaro_field.mulin(l[i], c);
			}
		}

		inline static void add_to(elem_t* l, const elem_t c, const elem_t* r, int len)
		{
			for(int i = 0; i < len; ++i)
			{
				givaro_field.axpyin(l[i], c, r[i]);
			}
		}

		inline static void add_to(elem_t* l, const elem_t* r, int len)
		{
			for(int i = 0; i < len; ++i)
			{
				givaro_field.addin(l[i], r[i]);
			}
		}

		inline static void substract_to(elem_t* l, const elem_t* r, int len)
		{
			for(int i = 0; i < len; ++i)
			{
				givaro_field.subin(l[i], r[i]);
			}
		}
	};

	template<std::size_t P, typename GivaroField>
	typename gf_p_givaro<P, GivaroField>::givaro_field_t
	gf_p_givaro<P, GivaroField>::givaro_field = GivaroField(P);

#ifdef __GIVARO_HAVE_INT128
	static const std::string digits = "0123456789";

	std::ostream& operator<<(std::ostream & os, int128_t value)
	{
		std::ostream::sentry s(os);
		if (s)
		{
			uint128_t temp = value < 0 ? -value : value;
			std::array<char, 128> buffer;
			auto it = buffer.end();
			do
			{
				--it;
				*it = digits[temp % 10];
				temp /= 10;
			} while(temp != 0);
			if (value < 0)
			{
				--it;
				*it = '-';
			}
			int len = buffer.end() - it;
			if ( os.rdbuf()->sputn(it, len) != len )
			{
				os.setstate( std::ios_base::badbit );
			}
		}

		return os;
	}

	std::ostream& operator<<(std::ostream & os, uint128_t value)
	{
		std::ostream::sentry s(os);
		if (s)
		{
			uint128_t temp = value;
			std::array<char, 128> buffer;
			auto it = buffer.end();
			do
			{
				--it;
				*it = digits[temp % 10];
				temp /= 10;
			} while(temp != 0);
			int len = buffer.end() - it;
			if ( os.rdbuf()->sputn(it, len) != len )
			{
				os.setstate( std::ios_base::badbit );
			}
		}

		return os;
	}
#endif

}

#endif
