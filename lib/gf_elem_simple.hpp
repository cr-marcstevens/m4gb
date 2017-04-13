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

#ifndef M4GB_GFELEM_HPP
#define M4GB_GFELEM_HPP

#include "detail.hpp"

#include <iostream>
#include <stdexcept>

namespace gb
{

	template<std::size_t gfsize>
	struct gf_elem_types {
		typedef typename detail::least_unsigned_integer_t< detail::nrbytes_t<gfsize-1>::value >::type elem_t;
		typedef typename detail::least_unsigned_integer_t< detail::nrbytes_t<(gfsize*2)-1>::value >::type dblelem_t;
		typedef typename detail::least_unsigned_integer_t< detail::nrbytes_t<(gfsize-1)*(gfsize-1)>::value >::type multelem_t;
		typedef typename detail::least_nonchar_unsigned_integer_t< detail::nrbytes_t<gfsize-1>::value >::type nonchar_elem_t;
	};

	struct tag_nocheck_t {};

	template<typename _GF>
	class gfelm
	{
	public:
		typedef _GF GF;
		typedef typename GF::elem_t elem_t;
		typedef typename GF::dblelem_t dblelem_t;
		typedef typename GF::multelem_t multelem_t;
		typedef typename GF::nonchar_elem_t nonchar_elem_t;

		elem_t v;

		gfelm()
			: v(0) 
		{
		}

		gfelm(const gfelm& _v) = default;

		gfelm& operator=(const gfelm& _v) = default;

		template<typename Int>
		gfelm(const Int _v) 
			: v((elem_t)(_v)) 
		{ 
			checkval(_v); 
		}

		template<typename Int>
		gfelm(const Int _v, tag_nocheck_t)
			: v((elem_t)(_v)) 
		{
		}

		template<typename Int>
		inline gfelm& operator=(const Int _v) 
		{ 
			checkval(_v);
			v = (elem_t)(_v); 
			return *this; 
		}

		inline gfelm& operator+=(const gfelm& r)
		{
			v = GF::add(v, r.v);
			return *this;
		}

		inline gfelm& operator-=(const gfelm& r)
		{
			v = GF::sub(v, r.v);
			return *this;
		}

		inline gfelm& operator*=(const gfelm& r)
		{
			v = GF::mult(v, r.v);
			return *this;
		}

		inline gfelm& operator/=(const gfelm& r)
		{
			v = GF::div(v, r.v);
			return *this;
		}

		inline gfelm& mul(const gfelm& r)
		{
			v = GF::mult(v, r.v);
			return *this;
		}

		inline gfelm& div(const gfelm& r)
		{
			v = GF::div(v, r.v);
			return *this;
		}

		inline gfelm& mul_nonzero(const gfelm& r)
		{
			v = GF::mult_nonzero(v, r.v);
			return *this;
		}

		inline gfelm& div_nonzero(const gfelm& r)
		{
			v = GF::div_nonzero(v, r.v);
			return *this;
		}

		inline bool operator!() const
		{
			return v == 0;
		}

		inline gfelm operator-() const
		{
			return gfelm(GF::negate(v), tag_nocheck_t());
		}

		template<typename Int>
		inline gfelm& operator+=(const Int r)
		{
			checkval(r);
			v = GF::add(v, (elem_t)(r));
			return *this;
		}

		template<typename Int>
		inline gfelm& operator-=(const Int r)
		{
			checkval(r);
			v = GF::sub(v, (elem_t)(r));
			return *this;
		}

		template<typename Int>
		inline gfelm& operator*=(const Int r)
		{
			checkval(r);
			v = GF::mult(v, (elem_t)(r));
			return *this;
		}

		template<typename Int>
		inline gfelm& operator/=(const Int r)
		{
			checkval(r);
			v = GF::div(v, (elem_t)(r));
			return *this;
		}

		template<typename Int>
		inline static void checkval(const Int r)
		{
			if ((std::size_t)(r) >= (std::size_t)(GF::gfsize))
				throw std::runtime_error("gfelem::checkval(): val >= gfsize");
		}

	};

	template<typename GF>
	inline bool operator==(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return l.v == r.v;
	}

	template<typename GF>
	inline bool operator!=(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return l.v != r.v;
	}

	template<typename GF>
	inline bool operator<(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return l.v < r.v;
	}

	template<typename GF>
	inline bool operator<=(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return l.v <= r.v;
	}

	template<typename GF>
	inline bool operator>(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return l.v > r.v;
	}

	template<typename GF>
	inline bool operator>=(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return l.v >= r.v;
	}

	template<typename GF, typename Int>
	inline bool operator==(const gfelm<GF>& l, const Int r)
	{
		return l.v == r;
	}

	template<typename GF, typename Int>
	inline bool operator!=(const gfelm<GF>& l, const Int r)
	{
		return l.v != r;
	}

	template<typename GF, typename Int>
	inline bool operator<(const gfelm<GF>& l, const Int r)
	{
		return l.v < r;
	}

	template<typename GF, typename Int>
	inline bool operator<=(const gfelm<GF>& l, const Int r)
	{
		return l.v <= r;
	}

	template<typename GF, typename Int>
	inline bool operator>(const gfelm<GF>& l, const Int r)
	{
		return l.v > r;
	}

	template<typename GF, typename Int>
	inline bool operator>=(const gfelm<GF>& l, const Int r)
	{
		return l.v >= r;
	}

	template<typename GF>
	inline gfelm<GF> operator+(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return gfelm<GF>(GF::add(l.v, r.v), tag_nocheck_t());
	}

	template<typename GF>
	inline gfelm<GF> operator-(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return gfelm<GF>(GF::sub(l.v, r.v), tag_nocheck_t());
	}

	template<typename GF>
	inline gfelm<GF> operator*(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return gfelm<GF>(GF::mult(l.v, r.v), tag_nocheck_t());
	}

	template<typename GF>
	gfelm<GF> operator/(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return gfelm<GF>(GF::div(l.v, r.v), tag_nocheck_t());
	}

	template<typename GF>
	inline gfelm<GF> mul(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return gfelm<GF>(GF::mult(l.v, r.v), tag_nocheck_t());
	}

	template<typename GF>
	inline gfelm<GF> div(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return gfelm<GF>(GF::div(l.v, r.v), tag_nocheck_t());
	}

	template<typename GF>
	inline gfelm<GF> mul_nonzero(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return gfelm<GF>(GF::mult_nonzero(l.v, r.v), tag_nocheck_t());
	}

	template<typename GF>
	inline gfelm<GF> div_nonzero(const gfelm<GF>& l, const gfelm<GF>& r)
	{
		return gfelm<GF>(GF::div_nonzero(l.v, r.v), tag_nocheck_t());
	}

	template<typename GF>
	inline std::ostream& operator<<(std::ostream& o, const gfelm<GF>& v)
	{
		return o << (typename GF::nonchar_elem_t)(v.v);
	}

	template<typename GF>
	inline std::istream& operator>>(std::istream& i, gfelm<GF>& v)
	{
		typename GF::nonchar_elem_t r;
		i >> r;
		v = r;
		return i;
	}

	/* vectorizable operations */
	template<typename GF>
	inline void mul_to(gfelm<GF>* l, const gfelm<GF> c, int len)
	{
		typedef typename GF::elem_t elem_t;
		GF::mul_to(reinterpret_cast<elem_t*>(l), c.v, len);
	}
	template<typename GF>
	inline void add_to(gfelm<GF>* l, const gfelm<GF> c, const gfelm<GF>* r, int len)
	{
		typedef typename GF::elem_t elem_t;
		GF::add_to(reinterpret_cast<elem_t*>(l), c.v, reinterpret_cast<const elem_t*>(r), len);
	}
	template<typename GF>
	inline void add_to(gfelm<GF>* l, const gfelm<GF>* r, int len)
	{
		typedef typename GF::elem_t elem_t;
		GF::add_to(reinterpret_cast<elem_t*>(l), reinterpret_cast<const elem_t*>(r), len);
	}
	template<typename GF>
	inline void substract_to(gfelm<GF>* l, const gfelm<GF>* r, int len)
	{
		typedef typename GF::elem_t elem_t;
		GF::substract_to(reinterpret_cast<elem_t*>(l), reinterpret_cast<const elem_t*>(r), len);
	}
	template<typename GF>
	inline void mul_to(std::vector< gfelm<GF> >& l, const gfelm<GF> c)
	{
		mul_to(&l[0], c, (int)(l.size()));
	}
	template<typename GF>
	inline void add_to(std::vector< gfelm<GF> >& l, const gfelm<GF> c, const std::vector< gfelm<GF> >& r)
	{
		if (l.size() < r.size())
			l.resize(r.size());
		add_to(&l[0], c, &r[0], (int)(r.size()));
	}
	template<typename GF>
	inline void add_to(std::vector< gfelm<GF> >& l, const std::vector< gfelm<GF> >& r)
	{
		if (l.size() < r.size())
			l.resize(r.size());
		add_to(&l[0], &r[0], (int)(r.size()));
	}
	template<typename GF>
	inline void substract_to(std::vector< gfelm<GF> >& l, const std::vector< gfelm<GF> >& r)
	{
		if (l.size() < r.size())
			l.resize(r.size());
		substract_to(&l[0], &r[0], (int)(r.size()));
	}

} // namespace gb

#endif // M4GB_GFELEM_HPP
