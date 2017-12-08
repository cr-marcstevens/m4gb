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
	template<std::size_t P>
	class gf_p_givaro
	{
	public:

#ifdef __GIVARO_HAVE_INT128
		static_assert(P <= 9223372036854775807u, "field size must be <= 9223372036854775807");
#else
		static_assert(P <= 4294967295, "field size must be <= 4294967295");
#endif

		static const std::size_t gfchar = P;
		static const std::size_t gfsize = P;
		static const std::size_t fieldchar = P;
		static const std::size_t fieldsize = P;

		typedef ::Givaro::Modular<uint8_t , uint8_t>  givmodular_uint8_uint8_t;   // P <= 2^4 - 1
		typedef ::Givaro::Modular<uint8_t , uint16_t> givmodular_uint8_uint16_t;  // P <= 2^7 - 1
		typedef ::Givaro::Modular<uint16_t, uint16_t> givmodular_uint16_uint16_t; // P <= 2^8 - 1
		typedef ::Givaro::Modular<uint16_t, uint32_t> givmodular_uint16_uint32_t; // P <= 2^15 - 1
		typedef ::Givaro::Modular<uint32_t, uint32_t> givmodular_uint32_uint32_t; // P <= 2^16 - 1
		typedef ::Givaro::Modular<uint32_t, uint64_t> givmodular_uint32_uint64_t; // P <= 2^31 - 1
		typedef ::Givaro::Modular<uint64_t, uint64_t> givmodular_uint64_uint64_t; // P <= 2^32 - 1

#ifdef __GIVARO_HAVE_INT128
		typedef ::Givaro::Modular<uint64_t, uint128_t> givmodular_uint64_uint128_t; // P <= 2^63 - 1
#endif

		typedef typename
		std::conditional<
			P <= 15, // 2^4 - 1
				givmodular_uint8_uint8_t,
				typename std::conditional<
				P <= 127, // 2^7 - 1
					givmodular_uint8_uint16_t,
					typename std::conditional<
					P <= 255, // 2^8 - 1
						givmodular_uint16_uint16_t,
						typename std::conditional<
						P <= 32767, // 2^15 - 1
							givmodular_uint16_uint32_t,
							typename std::conditional<
							P <= 65535, // 2^16 - 1
								givmodular_uint32_uint32_t,
								typename std::conditional<
								P <= 2147483647, // 2^31 - 1
									givmodular_uint32_uint64_t,
#ifdef __GIVARO_HAVE_INT128
									typename std::conditional<
									P <= 4294967295, // 2^32 - 1
										givmodular_uint64_uint64_t,
										givmodular_uint64_uint128_t
									>::type
#else
									givmodular_uint64_uint64_t
#endif
								>::type
							>::type
						>::type
					>::type
				>::type
		>::type givaro_field_t;

		typedef typename givaro_field_t::Element Element;
		typedef typename givaro_field_t::Compute_t Compute_t;
		typedef Element elem_t, nonchar_elem_t;
		typedef gfelm< gf_p_givaro<P> > gfelm_t;

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

	template<std::size_t P>
	typename gf_p_givaro<P>::givaro_field_t
	gf_p_givaro<P>::givaro_field = typename gf_p_givaro<P>::givaro_field_t(P);

}

#endif
