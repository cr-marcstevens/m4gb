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

#ifndef M4GB_MONOMIAL_DEGREVLEX_HPP
#define M4GB_MONOMIAL_DEGREVLEX_HPP


#include "detail.hpp"
#include "monomial_base.hpp"
#include "logger.hpp"

#include <map>
#include <vector>
#include <functional>
#include <iostream>
#include <stdlib.h>

#ifndef M4GB_MAX_INT_DEGREE
#define M4GB_MAX_INT_DEGREE 255
#endif

namespace gb
{

	template<std::size_t N, std::size_t D>
	struct monomial_degrevlex_less
	{
		typedef monomial_degrevlex_less<N, D> thisless;
		typedef typename monomial_traits<N, D>::var_greater var_greater;
		typedef monomial_dynamic_t<N, D, var_greater, thisless> monomial_t;
		typedef monomial_dynamic_t<N, D, var_greater, thisless> dynamic_monomial_t;
		typedef monomial_static_t<N, D, var_greater, thisless> static_monomial_t;

		typedef degrevlex_tag_t order_tag_t;

	private:
		template<typename Mo1, typename Mo2>
		static bool _less(Mo1& l, Mo2& r)
		{
			const unsigned degl = l.degree();
			const unsigned degr = r.degree();
			if (degl != degr)
				return degl < degr;
			if (degl == 0)
				return false;
			auto itl = l.begin();
			auto itr = r.begin();
			while (itl->first == itr->first && itl->second == itr->second)
			{
				if (++itr == r.end())
					return false;
				if (++itl == l.end())
					return true;
			}
			if (itl->first != itr->first)
				return itl->first > itr->first;
			return itl->second > itr->second;
		}
	public:

		static bool less(const static_monomial_t& l, const static_monomial_t& r) { return _less(l, r); }
		static bool less(const static_monomial_t& l, const        monomial_t& r) { return _less(l, r.data()); }
		static bool less(const        monomial_t& l, const static_monomial_t& r) { return _less(l.data(), r); }
		static bool less(const        monomial_t& l, const        monomial_t& r) { return _less(l.data(), r.data()); }

		template<typename int_type>
		static bool less(const monomial_int_t<N, D, var_greater, thisless, int_type>& l, const monomial_int_t<N, D, var_greater, thisless, int_type>& r)
		{
			return l < r;
		}

		template<typename int_type>
		static bool less(const monomial_int_t<N, D, var_greater, thisless, int_type>& lint, const static_monomial_t& r)
		{
			auto l = lint.decoder();
			return _less(l, r);
		}
		template<typename int_type>
		static bool less(const static_monomial_t& l, const monomial_int_t<N, D, var_greater, thisless, int_type>& rint)
		{
			auto r = rint.decoder();
			return _less(l, r);
		}

		template<typename int_type>
		static bool less(const monomial_int_t<N, D, var_greater, thisless, int_type>& lint, const monomial_t& r)
		{
			auto l = lint.decoder();
			return _less(l, r.data());
		}
		template<typename int_type>
		static bool less(const monomial_t& l, const monomial_int_t<N, D, var_greater, thisless, int_type>& rint)
		{
			auto r = rint.decoder();
			return less(l.data(), r);
		}

		template<typename Mo1, typename Mo2>
		bool operator()(const Mo1& l, const Mo2& r) const
		{
			return less(l, r);
		}
	};

	template<std::size_t N, std::size_t D>
	struct monomial_degrevlex_traits
		: monomial_traits < N, D >
	{
		typedef monomial_degrevlex_less<N, D> less_t;
		typedef typename less_t::monomial_t monomial_t;
		typedef typename less_t::dynamic_monomial_t dynamic_monomial_t;
		typedef typename less_t::static_monomial_t static_monomial_t;

		template<typename Int>
		struct monomial_int_type {
			typedef monomial_int_t<N, D, typename less_t::var_greater, less_t, Int> type;
		};
	};



	template<typename Ty> struct doublesizeint { };
	template<> struct doublesizeint < std::uint8_t > { typedef std::uint16_t type; };
	template<> struct doublesizeint < std::uint16_t > { typedef std::uint32_t type; };
	template<> struct doublesizeint < std::uint32_t > { typedef std::uint64_t type; };
	
	template<std::size_t N, std::size_t D, typename Int>
	struct monomial_degrevlex_intcodec
	{
		static const unsigned max_degree = D; 

		typedef typename monomial_degrevlex_traits<N, D>::monomial_t monomial_t;
		typedef typename monomial_degrevlex_traits<N, D>::static_monomial_t static_monomial_t;
		typedef typename monomial_t::pair_t pair_t;
		typedef typename monomial_t::sizetype sizetype;
		typedef Int int_type;


		struct static_data_t
		{
			int_type max_value;

			int_type L[D + 2];
			int_type T2[D + 1][N][D + 1];
			int_type T3[D + 1][N][N];

			static_data_t()
			{
				memset(T2, 0, sizeof(T2));
				memset(T3, 0, sizeof(T3));

				// temporary lookup table for multisetcoefficients
				std::vector< std::vector<int_type> > multisetcoef(N+1);
				for (unsigned i = 0; i <= N; ++i)
				{
					multisetcoef[i].resize(D + 1);
					for (unsigned d = 0; d <= D; ++d)
						multisetcoef[i][d] = detail::multiset_coefficient<int_type>(i,d);
				}

				L[0] = 0; L[1] = 1;
				for (unsigned i = 2; i <= D + 1; ++i)
					L[i] = L[i - 1] + multisetcoef[N][i - 1]; /* multiset_coefficient<cpp_int>(N, i - 1).convert_to<int_type>();*/
				max_value = L[D + 1] - 1;

				for (unsigned d = 0; d <= D; ++d)
				{
					for (unsigned i = 0; i < N; ++i)
					{
						// construct T2:
						for (unsigned e = 0; e <= d; ++e)
						{
							int_type c = 0;
							for (unsigned j = e + 1; j <= d; ++j)
								c += multisetcoef[i][d - j]; /* multiset_coefficient<cpp_int>(i, d - j).convert_to<int_type>();*/
							T2[d][i][e] = c;
						}

						// construct T3
						for (unsigned l = i; l < N; ++l)
						{
							int_type c = 0;
							for (unsigned j = 1; j <= d; ++j)
								if (i < l)
									c += multisetcoef[l - i][j] * multisetcoef[i + 1][d - j]; /* multiset_coefficient<int_type>(l - i, j) * multiset_coefficient<int_type>(i + 1, d - j);*/
							T3[d][l][i] = c;
						}
					}
				}
			}
		};
		static static_data_t static_data;

		static unsigned degree(const int_type& index)
		{
			unsigned d = 0;
			while (static_data.L[d + 1] <= index)
				++d;
			return d;
		}

		static int_type to_index(const static_monomial_t& m, unsigned d)
		{
			int_type v = static_data.L[d];
			unsigned l = N;
			for (auto it = m.begin(); it != m.end(); ++it)
			{
				v += static_data.T3[d][l - 1][it->first] + static_data.T2[d][it->first][it->second];
				d -= it->second;
				l = it->first;
			}
			return v;
		}
		static int_type to_index(const static_monomial_t& m)
		{
			unsigned d = m.degree();
			if (d > D)
				throw std::runtime_error("monomial_degrevlex_integer::to_index(): degree > D!");
			return to_index(m, d);
		}

		static unsigned decode_i(unsigned d, unsigned& l, int_type& v)
		{
			unsigned i = l;
			while (i > 0 && v >= static_data.T3[d][l][i - 1])
				--i;
			v -= static_data.T3[d][l][i];
			l = i - 1;
			return i;
		}
		static unsigned decode_e(unsigned& d, unsigned i, int_type& v)
		{
			unsigned e = 1;
			while (v < static_data.T2[d][i][e])
				++e;
			v -= static_data.T2[d][i][e];
			d -= e;
			return e;
		}
		static unsigned decode_e(int& d, unsigned i, int_type& v)
		{
			unsigned e = 1;
			while (v < static_data.T2[d][i][e])
				++e;
			v -= static_data.T2[d][i][e];
			d -= e;
			return e;
		}

		static void from_index(static_monomial_t& m, int_type v, unsigned d)
		{
			auto it = m._begin();
			v -= static_data.L[d];
			unsigned li = N - 1;
			while (d > 0)
			{
				it->first = decode_i(d, li, v);
				it->second = decode_e(d, it->first, v);
				++it;
			}
			m._size = (sizetype)(it - m._begin());
		}
		static static_monomial_t from_index(int_type v, unsigned d)
		{
			static_monomial_t tmp;
			from_index(tmp, v, d);
			return tmp;
		}
		static static_monomial_t from_index(int_type v)
		{
			static_monomial_t tmp;
			from_index(tmp, v, degree(v));
			return tmp;
		}

	};
	template<std::size_t N, std::size_t D, typename Int>
	typename monomial_degrevlex_intcodec<N, D, Int>::static_data_t monomial_degrevlex_intcodec<N, D, Int>::static_data;

	struct minimum_of_degree_tag {};
	struct maximum_of_degree_tag {};

	template<std::size_t N, std::size_t D, typename Int>
	class monomial_int_t < N, D, typename monomial_traits<N, D>::var_greater, monomial_degrevlex_less<N, D>, Int >
	{
	public:
		typedef typename monomial_traits<N, D>::var_greater vless;
		typedef monomial_degrevlex_less<N, D> mless;
		typedef monomial_static_t<N, D, vless, mless> monomial_static;
		typedef monomial_dynamic_t<N, D, vless, mless> monomial_dynamic;
		typedef typename mless::order_tag_t order_tag_t;

		typedef Int int_type;
		typedef typename monomial_traits<N, D>::varsizeinttype sizetype;
		typedef typename monomial_traits<N, D>::varinttype     varinttype;
		typedef typename monomial_traits<N, D>::expinttype     expinttype;

		typedef std::pair<varinttype, expinttype> pair_t;

		typedef pair_t*       pointer_t;
		typedef const pair_t* const_pointer_t;
		typedef std::reverse_iterator<pointer_t>       rev_pointer_t;
		typedef std::reverse_iterator<const_pointer_t> rev_const_pointer_t;

		static const std::size_t max_vars = N;
		static const std::size_t max_deg = D;

		typedef monomial_degrevlex_intcodec<N, D, Int> intcodec;

		static int_type max_value() { return intcodec::static_data.max_value; }
		static unsigned max_degree() { return intcodec::max_degree; }

		inline static expinttype add_exp(expinttype l, expinttype r)
		{
			return monomial_traits<N, D>::addexp(l, r);
		}
		inline static expinttype add_exp(expinttype l, expinttype r, bool& exponent_overflow)
		{
			return monomial_traits<N, D>::addexp(l, r, exponent_overflow);
		}
		inline static expinttype add_exp_nothrow(expinttype l, expinttype r)
		{
			return monomial_traits<N, D>::addexp_nothrow(l, r);
		}

		inline static expinttype sub_exp(expinttype l, expinttype r)
		{
			return monomial_traits<N, D>::subexp(l, r);
		}
		inline static expinttype sub_exp_nothrow(expinttype l, expinttype r)
		{
			return monomial_traits<N, D>::subexp_nothrow(l, r);
		}

	// private:
		int_type _m_int;
		vless _vless;
		mless _mless;
	public:

		monomial_int_t(int_type v = 0)
			: _m_int(v)
		{}

		~monomial_int_t()
		{}

		monomial_int_t(unsigned deg, minimum_of_degree_tag)
			: _m_int(intcodec::static_data.L[deg])
		{}

		monomial_int_t(unsigned deg, maximum_of_degree_tag)
			: _m_int(intcodec::static_data.L[deg+1]-1)
		{}

		monomial_int_t(const monomial_int_t& m)
			: _m_int(m._m_int)
		{}

		monomial_int_t(const monomial_static& m)
			: _m_int(intcodec::to_index(m))
		{}

		monomial_int_t(const monomial_dynamic& m)
			: _m_int(intcodec::to_index(m.data()))
		{}

		monomial_int_t(const pair_t& ve)
		{
			monomial_static tmp(ve);
			_m_int = intcodec::to_index(tmp);
		}

		template<typename Iter>
		monomial_int_t(Iter first, Iter last)
		{
			assign(first, last);
		}

		template<std::size_t N2, std::size_t D2, typename VLess2, typename MLess2>
		monomial_int_t(const monomial_static_t<N2, D2, VLess2, MLess2>& m)
		{
			assign(m.begin(), m.end());
		}

		template<std::size_t N2, std::size_t D2, typename VLess2, typename MLess2>
		monomial_int_t(const monomial_dynamic_t<N2, D2, VLess2, MLess2>& m)
		{
			assign(m.begin(), m.end());
		}

		monomial_int_t& operator=(const monomial_int_t& m)
		{
			_m_int = m._m_int;
			return *this;
		}
		
		monomial_int_t& operator=(const monomial_static& m)
		{
			_m_int = intcodec::to_index(m);
			return *this;
		}

		monomial_int_t& operator=(const monomial_dynamic& m)
		{
			_m_int = intcodec::to_index(m.data());
			return *this;
		}

		template<std::size_t N2, std::size_t D2, typename VLess2, typename MLess2>
		monomial_int_t& operator=(const monomial_static_t<N2, D2, VLess2, MLess2>& m)
		{
			assign(m.begin(), m.end());
			return *this;
		}

		template<std::size_t N2, std::size_t D2, typename VLess2, typename MLess2>
		monomial_int_t& operator=(const monomial_dynamic_t<N2, D2, VLess2, MLess2>& m)
		{
			assign(m.begin(), m.end());
			return *this;
		}

		template<typename Iter>
		void assign(Iter first, Iter last)
		{
			monomial_static tmp(first, last);
			_m_int = intcodec::to_index(tmp);
		}

		void clear()
		{
			_m_int = 0;
		}

		void swap(monomial_int_t& r)
		{
			std::swap(_m_int, r._m_int);
		}

		bool operator==(const monomial_int_t& r) const { return _m_int == r._m_int; }
		bool operator!=(const monomial_int_t& r) const { return _m_int != r._m_int; }
		bool operator< (const monomial_int_t& r) const { return _m_int <  r._m_int; }
		bool operator<=(const monomial_int_t& r) const { return _m_int <= r._m_int; }
		bool operator> (const monomial_int_t& r) const { return _m_int >  r._m_int; }
		bool operator>=(const monomial_int_t& r) const { return _m_int >= r._m_int; }

		bool operator==(const monomial_static& r) const 
		{
			auto itl = begin();
			auto itr = r.begin();
			for (; itl != end(); ++itl, ++itr)
			{
				if (itr == r.end() || *itl != *itr)
					return false;
			}
			return itr == r.end();
		}
		bool operator==(const monomial_dynamic& r) const
		{
			auto itl = begin();
			auto itr = r.begin();
			for (; itl != end(); ++itl, ++itr)
			{
				if (itr == r.end() || *itl != *itr)
					return false;
			}
			return itr == r.end();
		}
		bool operator!=(const monomial_static& r) const { return !(*this == r); }
		bool operator< (const monomial_static& r) const { return _mless(*this, r); }
		bool operator<=(const monomial_static& r) const { return !_mless(r, *this); }
		bool operator> (const monomial_static& r) const { return _mless(r, *this); }
		bool operator>=(const monomial_static& r) const { return !_mless(*this, r); }
		bool operator!=(const monomial_dynamic& r) const { return !(*this == r); }
		bool operator< (const monomial_dynamic& r) const { return _mless(*this, r); }
		bool operator<=(const monomial_dynamic& r) const { return !_mless(r, *this); }
		bool operator> (const monomial_dynamic& r) const { return _mless(r, *this); }
		bool operator>=(const monomial_dynamic& r) const { return !_mless(*this, r); }

		unsigned degree() const
		{
			return intcodec::degree(_m_int);
		}

		unsigned size() const
		{
			unsigned c = 0;
			for (auto it = begin(); it != end(); ++it)
				++c;
			return c;
		}
		unsigned count() const { return size(); }
		bool empty() const { return _m_int == 0; }
		int_type value() const { return _m_int; }

		unsigned get(varinttype var) const
		{
			for (auto ve : *this)
			{
				if (ve.first == var)
					return ve.second;
				if (ve.first < var)
					break;
			}
			return 0;
		}
		unsigned operator[](varinttype var) const
		{
			return get(var);
		}

		struct const_iterator_end {};
		class const_iterator
			: public std::iterator<std::forward_iterator_tag, pair_t>
		{
			int _d;
			unsigned _li;
			int_type _v;
			pair_t _ve;
		public:
			const_iterator()
				: _d(-1)
			{}
			const_iterator(const_iterator_end r)
				: _d(-1)
			{}
			const_iterator(int_type v)
				: _v(v)
			{
				_d = (int)(intcodec::degree(v));
				_v -= intcodec::static_data.L[_d];
				_li = N - 1;
				++*this;
			}
			const_iterator(int_type v, unsigned d)
				: _v(v), _d((int)(d))
			{
				_v -= intcodec::static_data.L[_d];
				_li = N - 1;
				++*this;
			}
			const_iterator& operator=(const const_iterator& r)
			{
				_d = r._d;
				_li = r._li;
				_v = r._v;
				_ve = r._ve;
				return *this;
			}
			const_iterator& operator++()
			{
				if (_d > 0)
				{
					_ve.first = intcodec::decode_i(_d, _li, _v);
					_ve.second = intcodec::decode_e(_d, _ve.first, _v);
				}
				else
					_d = -1;
				return *this;
			}
			const_iterator operator++(int) const
			{
				const_iterator tmp(*this);
				++*this;
				return tmp;
			}
			const pair_t& operator*() const { return _ve; }
			const pair_t* operator->() const { return &_ve; }
			bool operator==(const const_iterator& r) const
			{
				return _d == r._d;
			}
			bool operator!=(const const_iterator& r) const
			{
				return _d != r._d;
			}
			bool operator==(const const_iterator_end& r) const
			{
				return _d == -1;
			}
			bool operator!=(const const_iterator_end& r) const
			{
				return _d != -1;
			}
		};
		const_iterator begin() const { return const_iterator(_m_int); }
		const_iterator_end end() const { return const_iterator_end(); }

		struct decoder_type
		{
			unsigned d;
			int_type v;

			decoder_type()
				: d(0), v(0)
			{}
			decoder_type(int_type _v)
				: d(intcodec::degree(_v)), v(_v)
			{}

			unsigned degree() const { return d; }
			const_iterator begin() const { return const_iterator(v, d); }
			const_iterator_end end() const { return const_iterator_end(); }
		};
		decoder_type decoder() const { return decoder_type(_m_int); }

		template<typename Mo1, typename Mo2>
		void set_multiply(const Mo1& l, const Mo2& r, bool& exponent_has_decreased)
		{
			monomial_static tmp;
			tmp.set_multiply(l, r, exponent_has_decreased);
			_m_int = intcodec::to_index(tmp);
		}

		template<typename Mo1, typename Mo2>
		void set_multiply(const Mo1& l, const Mo2& r)
		{
			monomial_static tmp;
			tmp.set_multiply(l, r);
			_m_int = intcodec::to_index(tmp);
		}

		template<typename Mo1, typename Mo2>
		void set_divide(const Mo1& l, const Mo2& r)
		{
			monomial_static tmp;
			tmp.set_divide(l, r);
			_m_int = intcodec::to_index(tmp);
		}

		template<typename Mo1, typename Mo2>
		void set_lcm(const Mo1& l, const Mo2& r)
		{
			monomial_static tmp;
			tmp.set_lcm(l, r);
			_m_int = intcodec::to_index(tmp);
		}

		template<typename Mo1, typename Mo2>
		void set_gcd(const Mo1& l, const Mo2& r)
		{
			monomial_static tmp;
			tmp.set_gcd(l, r);
			_m_int = intcodec::to_index(tmp);
		}

		template<typename Mo>
		bool divides(const Mo& r) const
		{
			auto it1 = begin();
			auto it2 = r.begin();
			while (true)
			{
				if (it1 == end())
					return true;
				if (it2 == r.end())
					return false;
				if (it1->first == it2->first)
				{
					if (it1->second > it2->second)
						return false;
					++it1; ++it2;
				}
				else if (_vless(it1->first, it2->first))
					return false;
				else
					++it2;
			}
		}

		template<typename Mo>
		bool disjoint(const Mo& r) const
		{
			if (empty() || r.empty())
				return true;
			auto it1 = begin();
			auto it2 = r.begin();
			while (true)
			{
				if (it1->first == it2->first)
					return false;
				else if (_vless(it1->first, it2->first))
				{
					if (++it1 == end())
						return true;
				}
				else
				{
					if (++it2 == end())
						return true;
				}
			}
		}

		template<typename Mo>
		bool operator|(const Mo& r) const
		{
			return divides(r);
		}

		monomial_int_t& operator*=(const monomial_static& r)
		{
			if (!r.empty())
			{
				monomial_static tmp;
				tmp.set_multiply(*this, r);
				_m_int = intcodec::to_index(tmp);
			}
			return *this;
		}
		monomial_int_t& operator*=(const monomial_dynamic& r)
		{
			if (!r.empty())
			{
				monomial_static tmp;
				tmp.set_multiply(*this, r.data());
				_m_int = intcodec::to_index(tmp);
			}
			return *this;
		}
		monomial_int_t& operator*=(const monomial_int_t& r)
		{
			if (!r.empty())
			{
				monomial_static tmp;
				tmp.set_multiply(*this, r);
				_m_int = intcodec::to_index(tmp);
			}
			return *this;
		}

		monomial_static operator*(const monomial_static& r) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r);
			return tmp;
		}
		monomial_static operator*(const monomial_dynamic& r) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r.data());
			return tmp;
		}
		monomial_static operator*(const monomial_int_t& r) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r);
			return tmp;
		}

		monomial_static multiply(const monomial_static& r, bool& exponent_has_decreased) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r, exponent_has_decreased);
			return tmp;
		}
		monomial_static multiply(const monomial_dynamic& r, bool& exponent_has_decreased) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r.data(), exponent_has_decreased);
			return tmp;
		}
		template<typename int_type>
		monomial_static multiply(const monomial_int_t& r, bool& exponent_has_decreased) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r, exponent_has_decreased);
			return tmp;
		}

		monomial_int_t& operator/=(const monomial_static& r)
		{
			if (!r.empty())
			{
				monomial_static tmp;
				tmp.set_divide(*this, r);
				_m_int = intcodec::to_index(tmp);
			}
			return *this;
		}
		monomial_int_t& operator/=(const monomial_dynamic& r)
		{
			if (!r.empty())
			{
				monomial_static tmp;
				tmp.set_divide(*this, r.data());
				_m_int = intcodec::to_index(tmp);
			}
			return *this;
		}
		monomial_int_t& operator/=(const monomial_int_t& r)
		{
			if (!r.empty())
			{
				monomial_static tmp;
				tmp.set_divide(*this, r);
				_m_int = intcodec::to_index(tmp);
			}
			return *this;
		}

		monomial_static operator/(const monomial_static& r) const
		{
			monomial_static tmp;
			tmp.set_divide(*this, r);
			return tmp;
		}
		monomial_static operator/(const monomial_dynamic& r) const
		{
			monomial_static tmp;
			tmp.set_divide(*this, r.data());
			return tmp;
		}
		monomial_static operator/(const monomial_int_t& r) const
		{
			monomial_static tmp;
			tmp.set_divide(*this, r);
			return tmp;
		}

	};

	template<std::size_t N, std::size_t IntSize>
	struct monomial_degrevlex_traits_int_helper
	{
		typedef typename detail::least_unsigned_integer_t<IntSize>::type int_type;
		static const std::size_t D = detail::max_degree_fits_int_t<N,M4GB_MAX_INT_DEGREE,int_type>::value;
		typedef monomial_degrevlex_traits<N, D> base_traits;
		typedef monomial_int_t<N, D, typename monomial_traits<N, D>::var_greater, monomial_degrevlex_less<N, D>,  int_type> int_monomial_t;
	};

	template<std::size_t N, std::size_t IntSize>
	struct monomial_degrevlex_traits_int
		: monomial_degrevlex_traits_int_helper<N,IntSize>::base_traits
	{
		typedef typename monomial_degrevlex_traits_int_helper<N,IntSize>::int_monomial_t int_monomial_t;
	};

	template<std::size_t N>
	struct monomial_degrevlex_traits_uint8
		: monomial_degrevlex_traits_int<N, 1>
	{
	};

	template<std::size_t N>
	struct monomial_degrevlex_traits_uint16
		: monomial_degrevlex_traits_int<N, 2>
	{
	};

	template<std::size_t N>
	struct monomial_degrevlex_traits_uint32
		: monomial_degrevlex_traits_int<N, 4>
	{
	};

	template<std::size_t N>
	struct monomial_degrevlex_traits_uint64
		: monomial_degrevlex_traits_int<N, 8>
	{
	};

} // namespace gb

#endif // M4GB_MONOMIAL_DEGREVLEX_HPP
