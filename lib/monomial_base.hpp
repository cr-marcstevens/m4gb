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

#ifndef M4GB_MONOMIAL_BASE_HPP
#define M4GB_MONOMIAL_BASE_HPP

#include "detail.hpp"

#include <map>
#include <vector>
#include <functional>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace gb
{

	struct unordered_tag_t {};
	struct degrevlex_tag_t {};

	// set default types to store monomials over N variables X_0, ..., X_(N-1)
	template<std::size_t N, std::size_t D>
	struct monomial_traits
	{
		static const std::size_t max_vars = N;
		static const std::size_t max_deg = D;

		typedef typename gb::detail::least_unsigned_integer_t< gb::detail::nrbytes_t<N>::value >::type             varsizeinttype;
		typedef typename gb::detail::least_unsigned_integer_t< gb::detail::nrbytes_t<N - 1>::value >::type         varinttype;
		typedef typename gb::detail::least_nonchar_unsigned_integer_t< gb::detail::nrbytes_t<N - 1>::value >::type nonchar_varinttype;

		typedef typename gb::detail::least_unsigned_integer_t< gb::detail::nrbytes_t<D>::value >::type         expinttype;
		typedef typename gb::detail::least_nonchar_unsigned_integer_t< gb::detail::nrbytes_t<D>::value >::type nonchar_expinttype;
		typedef typename gb::detail::least_unsigned_integer_t< gb::detail::nrbytes_t<2 * D>::value >::type     dblexpinttype;

		inline static expinttype addexp(const expinttype l, const expinttype r)
		{
			if (l+r < l)
				throw std::runtime_error("monomial_traits<N,P>::addexp(): exponent addition overflow!");
			return l+r;
		}
		inline static expinttype addexp_nothrow(const expinttype l, const expinttype r)
		{
			return l+r;
		}
		inline static expinttype addexp(const expinttype l, const expinttype r, bool& exponent_overflow)
		{
			exponent_overflow |= (l+r < l);
			return l+r;
		}

		// subtract exponents, throw for negative result
		inline static expinttype subexp(const expinttype l, const expinttype r)
		{
			if (r > l)
				throw std::runtime_error("monomial_traits<N,P>::subexp(): negative exponent not allowed!");
			return l - r;
		}
		inline static expinttype subexp_nothrow(const expinttype l, const expinttype r)
		{
			return l - r;
		}

		struct var_less    
		{ 
			inline bool operator()(varinttype l, varinttype r) const 
			{ return l < r; } 

			template<typename T>
			inline bool operator()(varinttype l, std::pair<varinttype, T>& r) const 
			{ return l < r.first; }

			template<typename T>
			inline bool operator()(const std::pair<varinttype, T>& l, varinttype r) const 
			{ return l.first < r; }

			template<typename T>
			inline bool operator()(const std::pair<varinttype, T>& l, std::pair<varinttype, T>& r) const 
			{ return l.first < r.first; }
		};

		struct var_greater 
		{ 
			inline bool operator()(varinttype l, varinttype r) const 
			{ return l > r; }
			
			template<typename T>
			inline bool operator()(varinttype l, std::pair<varinttype, T>& r) const 
			{ return l > r.first; }

			template<typename T>
			inline bool operator()(const std::pair<varinttype, T>& l, varinttype r) const 
			{ return l.first > r; }

			template<typename T>
			inline bool operator()(const std::pair<varinttype, T>& l, std::pair<varinttype, T>& r) const 
			{ return l.first > r.first; }
		};

		struct exp_less    
		{ 
			inline bool operator()(expinttype l, expinttype r) const { return l < r; } 
		};
		struct exp_greater 
		{ 
			inline bool operator()(expinttype l, expinttype r) const { return l > r; } 
		};
	};

	struct throw_less
	{
		typedef unordered_tag_t order_tag_t;

		template<typename X>
		inline bool operator()(const X& l, const X& r) const
		{
			throw std::runtime_error("throw_less");
		}
	};

	template<std::size_t N, std::size_t D, typename VLess = typename monomial_traits<N, D>::var_less, typename MLess = throw_less>
	class monomial_static_t;

	template<std::size_t N, std::size_t D, typename VLess = typename monomial_traits<N, D>::var_less, typename MLess = throw_less>
	class monomial_dynamic_t;

	template<std::size_t N, std::size_t D, typename VLess = typename monomial_traits<N, D>::var_less, typename MLess = throw_less, typename int_type = std::uint32_t>
	class monomial_int_t
	{};

	template<std::size_t N, std::size_t D, typename VLess, typename MLess>
	class monomial_static_t
	{
		friend class monomial_dynamic_t<N,D,VLess,MLess>;
		template<std::size_t N2, std::size_t D2, typename VL2, typename ML2, typename Int>
		friend class monomial_int_t;
	public:
		typedef monomial_static_t<N, D, VLess, MLess> monomial_static;
		typedef monomial_dynamic_t<N, D, VLess, MLess> monomial_dynamic;

		typedef VLess vless;
		typedef MLess mless;
		typedef typename MLess::order_tag_t order_tag_t;

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

		inline static expinttype add_exp(expinttype l, expinttype r)
		{
			return monomial_traits<N, D>::addexp(l, r);
		}
		inline static expinttype add_exp_nothrow(expinttype l, expinttype r)
		{
			return monomial_traits<N, D>::addexp_nothrow(l, r);
		}
		inline static expinttype add_exp(expinttype l, expinttype r, bool& exponent_overflow)
		{
			return monomial_traits<N, D>::addexp(l, r, exponent_overflow);
		}

		inline static expinttype sub_exp(expinttype l, expinttype r)
		{
			return monomial_traits<N, D>::subexp(l, r);
		}
		inline static expinttype sub_exp_nothrow(expinttype l, expinttype r)
		{
			return monomial_traits<N, D>::subexp_nothrow(l, r);
		}

//	private:
		VLess _vless;
		MLess _mless;
		sizetype _size;
		pair_t    _data[N];
//		std::size_t _datasize() const { return (std::size_t)(_size)* sizeof(pair_t); }

	public:

		monomial_static_t()
			: _size(0)
		{}

		~monomial_static_t()
		{}

		monomial_static_t(const monomial_static& m)
		{
			_size = m.size();
			memcpy(_data, m._data, (std::size_t)(_size)*sizeof(pair_t));
		}

		monomial_static_t(const monomial_dynamic& m)
		{
			_size = m.size();
			memcpy(_data, m.data()._data, (std::size_t)(_size)*sizeof(pair_t));
		}

		template<typename int_type>
		monomial_static_t(const monomial_int_t<N, D, VLess, MLess, int_type>& m)
			: _size(0)
		{
			for (auto it = m.begin(); it != m.end(); ++it)
				_data[_size++] = *it;
		}

		monomial_static_t(const pair_t& ve)
		{
			_size = 1;
			_data[0] = ve;
		}

		template<typename Iter>
		monomial_static_t(Iter first, Iter last)
		{
			assign(first, last);
		}

		template<std::size_t N2, std::size_t D2, typename VLess2, typename MLess2>
		monomial_static_t(const monomial_static_t<N2, D2, VLess2, MLess2>& m)
		{
			assign(m.begin(), m.end());
		}

		template<std::size_t N2, std::size_t D2, typename VLess2, typename MLess2>
		monomial_static_t(const monomial_dynamic_t<N2, D2, VLess2, MLess2>& m)
		{
			assign(m.begin(), m.end());
		}

		monomial_static_t& operator=(const monomial_static& m)
		{
			_size = m.size();
			memcpy(_data, m._data, (std::size_t)(_size)*sizeof(pair_t));
			return *this;
		}

		monomial_static_t& operator=(const monomial_dynamic& m)
		{
			_size = m.size();
			memcpy(_data, m.data()._data, (std::size_t)(_size)*sizeof(pair_t));
			return *this;
		}

		template<typename int_type>
		monomial_static_t& operator=(const monomial_int_t<N, D, VLess, MLess, int_type>& m)
		{
			_size = 0;
			for (auto it = m.begin(); it != m.end(); ++it)
				_data[_size++] = *it;
			return *this;
		}

		template<std::size_t N2, std::size_t D2, typename VLess2, typename MLess2>
		monomial_static_t& operator=(const monomial_static_t<N2,D2,VLess2,MLess2>& m)
		{
			assign(m.begin(), m.end());
			return *this;
		}

		template<std::size_t N2, std::size_t D2, typename VLess2, typename MLess2>
		monomial_static_t& operator=(const monomial_dynamic_t<N2,D2,VLess2,MLess2>& m)
		{
			assign(m.begin(), m.end());
			return *this;
		}

		template<typename Iter>
		void assign(Iter first, Iter last)
		{
			clear();
			std::size_t size = std::distance(first, last);
			if (size == 0)
				return;
			if (size > N)
				throw std::runtime_error("monomial_static_t::assign(): too many vars!");
			_size = (sizetype)(size);
			pair_t* it = _begin();
			for (; first != last; ++first)
			{
				const std::size_t i = first->first;
				const std::size_t e = first->second;
				if (i >= N)
					throw std::runtime_error("monomial_static_t::assign(): varint out of bounds!");
				if (e > D)
					throw std::runtime_error("monomial_static_t::assign(): expint out of bounds!");
				if (e == 0)
					continue;
				it->first = (varinttype)(i);
				it->second = (expinttype)(e);
				++it;
			}
			if (it != _end())
				_size = (sizetype)(it - _begin());
			std::sort(_begin(), _end(), _vless);
			for (it = _data + 1; it != _data + _size; ++it)
				if ((it - 1)->first == it->first)
					throw std::runtime_error("monomial_static_t::assign(Iter first, Iter last): expected unique variables");
		}

		void clear()
		{
			_size = 0;
		}

		void swap(monomial_static& r)
		{
			if (size() < r.size())
			{
				monomial_static tmp(*this);
				*this = r;
				r = tmp;
			}
			else
			{
				monomial_static tmp(r);
				r = *this;
				*this = tmp;
			}
		}

		const char* begin_ptr() const { return reinterpret_cast<const char*>(this); }
		const char* end_ptr()   const { return reinterpret_cast<const char*>(_data + _size); }
		const_pointer_t begin() const { return _data; }
		const_pointer_t end()   const { return _data + _size; }
		rev_const_pointer_t rbegin() const { return rev_const_pointer_t(end()); }
		rev_const_pointer_t rend()   const { return rev_const_pointer_t(begin()); }

		char*       _begin_ptr()   { return reinterpret_cast<char*>(this); }
		char*       _end_ptr()     { return reinterpret_cast<char*>(_data + _size); }
	//private:
		pointer_t   _begin()       { return _data; }
		pointer_t   _end()         { return _data + _size; }
	public:

		bool operator==(const monomial_static& r) const
		{
			if (_size != r._size)
				return false;
			return std::equal(begin(), end(), r.begin());
		}
		bool operator!=(const monomial_static& r) const { return !(*this == r); }

		bool operator==(const monomial_dynamic& r) const
		{
			if (_size != r.size())
				return false;
			return std::equal(begin(), end(), r.begin());
		}
		bool operator!=(const monomial_dynamic& r) const { return !(*this == r); }

		bool operator< (const monomial_static& r) const { return _mless(*this, r); }
		bool operator> (const monomial_static& r) const { return _mless(r, *this); }
		bool operator>=(const monomial_static& r) const { return !_mless(*this, r); }
		bool operator<=(const monomial_static& r) const { return !_mless(r, *this); }

		bool operator< (const monomial_dynamic& r) const { return _mless(*this, r); }
		bool operator> (const monomial_dynamic& r) const { return _mless(r, *this); }
		bool operator>=(const monomial_dynamic& r) const { return !_mless(*this, r); }
		bool operator<=(const monomial_dynamic& r) const { return !_mless(r, *this); }

		template<typename int_type>
		bool operator==(const monomial_int_t<N, D, VLess, MLess, int_type>& m) const
		{
			return m == *this;
		}
		template<typename int_type>
		bool operator!=(const monomial_int_t<N, D, VLess, MLess, int_type>& m) const
		{
			return m != *this;
		}
		template<typename int_type>
		bool operator< (const monomial_int_t<N, D, VLess, MLess, int_type>& r) const { return _mless(*this, r); }
		template<typename int_type>
		bool operator> (const monomial_int_t<N, D, VLess, MLess, int_type>& r) const { return _mless(r, *this); }
		template<typename int_type>
		bool operator>=(const monomial_int_t<N, D, VLess, MLess, int_type>& r) const { return !_mless(*this, r); }
		template<typename int_type>
		bool operator<=(const monomial_int_t<N, D, VLess, MLess, int_type>& r) const { return !_mless(r, *this); }

		unsigned degree() const
		{
			unsigned d = 0;
			for (auto it = begin(); it != end(); ++it)
				d += (unsigned)(it->second);
			return d;
		}

		unsigned size()  const { return _size; }
		unsigned count() const { return _size; }
		bool     empty() const { return _size == 0; }


		unsigned get(varinttype var) const
		{
			auto it = std::lower_bound<const_pointer_t>(begin(), end(), var, _vless);
			if (it != end() && it->first == var)
				return it->second;
			return 0;
		}
		unsigned operator[](varinttype var) const
		{
			return get(var);
		}

		template<typename Mo1, typename Mo2>
		void set_multiply(const Mo1& l, const Mo2& r, bool& exponent_overflow)
		{
			if (l.empty())
			{
				*this = r;
				return;
			}
			if (r.empty())
			{
				*this = l;
				return;
			}
			auto it = _begin();
			auto it1 = l.begin();
			auto it2 = r.begin();
			while (true)
			{
				if (it1->first == it2->first)
				{
					it->first = it1->first;
					it->second = add_exp(it1->second, it2->second, exponent_overflow);
					++it; ++it1; ++it2;
					if (it1 == l.end() || it2 == r.end())
						break;
				}
				else if (_vless(it1->first, it2->first))
				{
					*it = *it1;
					++it; ++it1;
					if (it1 == l.end())
						break;
				}
				else
				{
					*it = *it2;
					++it; ++it2;
					if (it2 == r.end())
						break;
				}
			}
			for (; it1 != l.end(); ++it, ++it1)
				*it = *it1;
			for (; it2 != r.end(); ++it, ++it2)
				*it = *it2;

			_size = (sizetype)(it - _begin());
		}

		template<typename Mo1, typename Mo2>
		void set_multiply(const Mo1& l, const Mo2& r)
		{
			if (l.empty())
			{
				*this = r;
				return;
			}
			if (r.empty())
			{
				*this = l;
				return;
			}
			auto it = _begin();
			auto it1 = l.begin();
			auto it2 = r.begin();
			while (true)
			{
				if (it1->first == it2->first)
				{
					it->first = it1->first;
					it->second = add_exp(it1->second, it2->second);
					++it; ++it1; ++it2;
					if (it1 == l.end() || it2 == r.end())
						break;
				}
				else if (_vless(it1->first, it2->first))
				{
					*it = *it1;
					++it; ++it1;
					if (it1 == l.end())
						break;
				}
				else
				{
					*it = *it2;
					++it; ++it2;
					if (it2 == r.end())
						break;
				}
			}
			for (; it1 != l.end(); ++it, ++it1)
				*it = *it1;
			for (; it2 != r.end(); ++it, ++it2)
				*it = *it2;

			_size = (sizetype)(it - _begin());
		}

		template<typename Mo1, typename Mo2>
		void set_divide(const Mo1& l, const Mo2& r)
		{
			if (r.empty())
			{
				*this = l;
				return;
			}
			if (l.empty())
				throw std::runtime_error("monomial_static_t::set_divide(l,r): right-hand monomial does not divide left-hand monomial!");
			auto it = _begin();
			auto it1 = l.begin();
			auto it2 = r.begin();
			while (true)
			{
				if (it1->first == it2->first)
				{
					if (it1->second > it2->second)
					{
						it->first = it1->first;
						it->second = it1->second - it2->second;
						++it;
					} 
					else if (it1->second != it2->second)
						throw std::runtime_error("monomial_static_t::set_divide(l,r): right-hand monomial does not divide left-hand monomial!");
					++it1; ++it2;
					if (it2 == r.end())
						break;
					if (it1 == l.end())
						throw std::runtime_error("monomial_static_t::set_divide(l,r): right-hand monomial does not divide left-hand monomial!");
				}
				else if (_vless(it1->first, it2->first))
				{
					*it = *it1; ++it; ++it1;
					if (it1 == l.end())
						throw std::runtime_error("monomial_static_t::set_divide(l,r): right-hand monomial does not divide left-hand monomial!");
				}
				else
					throw std::runtime_error("monomial_static_t::set_divide(l,r): right-hand monomial does not divide left-hand monomial!");
			}
			for (; it1 != l.end(); ++it, ++it1)
				*it = *it1;
			_size = (sizetype)(it - _begin());
		}
		
		template<typename Mo1, typename Mo2>
		void set_lcm(const Mo1& l, const Mo2& r)
		{
			if (l.empty())
			{
				*this = r;
				return;
			}
			if (r.empty())
			{
				*this = l;
				return;
			}
			auto it = _begin();
			auto it1 = l.begin();
			auto it2 = r.begin();
			while (true)
			{
				if (it1->first == it2->first)
				{
					it->first = it1->first;
					it->second = it1->second > it2->second ? it1->second : it2->second;
					++it; ++it1; ++it2;
					if (it1 == l.end() || it2 == r.end())
						break;
				}
				else if (_vless(it1->first, it2->first))
				{
					*it = *it1;
					++it; ++it1;
					if (it1 == l.end())
						break;
				}
				else
				{
					*it = *it2;
					++it; ++it2;
					if (it2 == r.end())
						break;
				}
			}
			for (; it1 != l.end(); ++it, ++it1)
				*it = *it1;
			for (; it2 != r.end(); ++it, ++it2)
				*it = *it2;

			_size = (sizetype)(it - _begin());
		}

		template<typename Mo1, typename Mo2>
		void set_gcd(const Mo1& l, const Mo2& r)
		{
			if (l.empty() || r.empty())
			{
				clear();
				return;
			}
			auto it = _begin();
			auto it1 = l.begin();
			auto it2 = r.begin();
			while (true)
			{
				if (it1->first == it2->first)
				{
					if (it1->second < it2->second)
					{
						if (it1->second != 0)
						{
							it->first = it1->first;
							it->second = it1->second;
							++it;
						}
					}
					else
					{
						if (it2->second != 0)
						{
							it->first = it2->first;
							it->second = it2->second;
						}
					}
					++it; ++it1; ++it2;
					if (it1 == l.end() || it2 == r.end())
						break;
				}
				else if (_vless(it1->first, it2->first))
				{
					++it1;
					if (it1 == l.end())
						break;
				}
				else
				{
					++it2;
					if (it2 == r.end())
						break;
				}
			}
			_size = (sizetype)(it - _begin());
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

		monomial_static_t& operator*=(const monomial_static& r)
		{
			if (!r.empty())
			{
				monomial_static_t tmp(*this);
				set_multiply(tmp, r);
			}
			return *this;
		}
		monomial_static_t& operator*=(const monomial_dynamic& r)
		{
			if (!r.empty())
			{
				monomial_static_t tmp(*this);
				set_multiply(tmp, r._data());
			}
			return *this;
		}
		template<typename int_type>
		monomial_static_t& operator*=(const monomial_int_t<N, D, VLess, MLess, int_type>& r)
		{
			if (!r.empty())
			{
				monomial_static_t tmp(*this);
				set_multiply(tmp, r);
			}
			return *this;
		}

		monomial_static_t operator*(const monomial_static& r) const
		{
			monomial_static_t tmp;
			tmp.set_multiply(*this, r);
			return tmp;
		}
		monomial_static_t operator*(const monomial_dynamic& r) const
		{
			monomial_static_t tmp;
			tmp.set_multiply(*this, r.data());
			return tmp;
		}
		template<typename int_type>
		monomial_static_t operator*(const monomial_int_t<N, D, VLess, MLess, int_type>& r) const
		{
			monomial_static_t tmp;
			tmp.set_multiply(*this, r);
			return tmp;
		}

		monomial_static multiply(const monomial_static& r, bool& exponent_overflow) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r, exponent_overflow);
			return tmp;
		}
		monomial_static multiply(const monomial_dynamic& r, bool& exponent_overflow) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r.data(), exponent_overflow);
			return tmp;
		}
		template<typename int_type>
		monomial_static_t multiply(const monomial_int_t<N, D, VLess, MLess, int_type>& r, bool& exponent_overflow) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r, exponent_overflow);
			return tmp;
		}

		monomial_static_t& operator/=(const monomial_static& r)
		{
			if (!r.empty())
				set_divide(*this, r);
			return *this;
		}
		monomial_static_t& operator/=(const monomial_dynamic& r)
		{
			if (!r.empty())
				set_divide(*this, r._data());
			return *this;
		}
		template<typename int_type>
		monomial_static_t& operator/=(const monomial_int_t<N, D, VLess, MLess, int_type>& r)
		{
			if (!r.empty())
				set_divide(*this, r);
			return *this;
		}

		monomial_static_t operator/(const monomial_static& r) const
		{
			monomial_static_t tmp;
			tmp.set_divide(*this, r);
			return tmp;
		}
		monomial_static_t operator/(const monomial_dynamic& r) const
		{
			monomial_static_t tmp;
			tmp.set_divide(*this, r.data());
			return tmp;
		}
		template<typename int_type>
		monomial_static_t operator/(const monomial_int_t<N, D, VLess, MLess, int_type>& r) const
		{
			monomial_static_t tmp;
			tmp.set_divide(*this, r);
			return tmp;
		}

	}; // monomial_static_t



	template<std::size_t N, std::size_t D, typename VLess, typename MLess>
	class monomial_dynamic_t
	{
		friend class monomial_static_t<N, D, VLess, MLess>;
	public:
		typedef monomial_static_t<N, D, VLess, MLess> monomial_static;
		typedef monomial_dynamic_t<N, D, VLess, MLess> monomial_dynamic;

		typedef VLess vless;
		typedef MLess mless;
		typedef typename MLess::order_tag_t order_tag_t;

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

		static monomial_static _empty_monomial;

		monomial_dynamic_t()
			: _ptr(nullptr)
		{
		}
		~monomial_dynamic_t()
		{
			if (_ptr != nullptr)
				free(_ptr);
		}

		monomial_dynamic_t(const monomial_dynamic_t& r)
			: _ptr(nullptr)
		{
			if (!r.empty())
			{
				_data_malloc(r.size());
				_data() = r._data();
			}
		}

		monomial_dynamic_t(monomial_dynamic_t&& r)
			: _ptr(r._ptr)
		{
			r._ptr = nullptr;
		}

		monomial_dynamic_t(const monomial_static& r)
			: _ptr(nullptr)
		{
			if (!r.empty())
			{
				_data_malloc(r.size());
				_data() = r;
			}
		}

		template<typename int_type>
		monomial_dynamic_t(const monomial_int_t<N, D, VLess, MLess, int_type>& r)
			: _ptr(nullptr)
		{
			if (!r.empty())
			{
				monomial_static tmp(r);
				*this = tmp;
			}
		}

		monomial_dynamic_t(const pair_t& ve)
			: _ptr(nullptr)
		{
			_data_malloc(1);
			_data()._size = 1;
			_data()._data[0] = ve;
		}

		template<typename Iter>
		monomial_dynamic_t(Iter first, Iter last)
			: _ptr(nullptr)
		{
			assign(first, last);
		}

		template<std::size_t N2, std::size_t D2, typename VL2, typename ML2>
		monomial_dynamic_t(const monomial_dynamic_t<N2, D2, VL2, ML2>& r)
			: _ptr(nullptr)
		{
			assign(r.begin(), r.end());
		}

		template<std::size_t N2, std::size_t D2, typename VL2, typename ML2>
		monomial_dynamic_t(const monomial_static_t<N2, D2, VL2, ML2>& r)
			: _ptr(nullptr)
		{
			assign(r.begin(), r.end());
		}

		monomial_dynamic_t& operator=(const monomial_dynamic_t& r)
		{
			if (r.empty())
			{
				clear();
			} 
			else if (empty())
			{
				_data_malloc(r.size());
				_data() = r._data();
			}
			else 
			{
				if (size() != r.size())
					_data_realloc(r.size());
				_data() = r._data();
			}
			return *this;
		}

		monomial_dynamic_t& operator=(monomial_dynamic_t&& r)
		{
			swap(r);
			return *this;
		}

		monomial_dynamic_t& operator=(const monomial_static& r)
		{
			if (r.empty())
			{
				clear();
			}
			else if (empty())
			{
				_data_malloc(r.size());
				_data() = r;
			}
			else 
			{
				if (size() != r.size())
					_data_realloc(r.size());
				_data() = r;
			}
			return *this;
		}

		template<typename int_type>
		monomial_dynamic_t& operator=(const monomial_int_t<N, D, VLess, MLess, int_type>& r)
		{
			if (r.empty())
			{
				clear();
			}
			else
			{
				monomial_static tmp(r);
				*this = tmp;
			}
			return *this;
		}

		template<std::size_t N2, std::size_t D2, typename VL2, typename ML2>
		monomial_dynamic_t& operator=(const monomial_dynamic_t<N2, D2, VL2, ML2>& r)
		{
			assign(r.begin(), r.end());
			return *this;
		}

		template<std::size_t N2, std::size_t D2, typename VL2, typename ML2>
		monomial_dynamic_t& operator=(const monomial_static_t<N2, D2, VL2, ML2>& r)
		{
			assign(r.begin(), r.end());
			return *this;
		}
		
		template<typename Iter>
		void assign(Iter first, Iter last)
		{
			clear();
			monomial_static tmp(first, last);
			*this = tmp;
		}

		void clear()
		{
			if (!empty())
			{
				free(_ptr);
				_ptr = nullptr;
			}
		}

		void swap(monomial_dynamic_t& r)
		{
			std::swap(_ptr, r._ptr);
		}


		const_pointer_t begin() const { return data().begin(); }
		const_pointer_t end()   const { return data().end(); }
		rev_const_pointer_t rbegin() const { return rev_const_pointer_t(end()); }
		rev_const_pointer_t rend()   const { return rev_const_pointer_t(begin()); }

		bool operator==(const monomial_dynamic& r) const
		{
			if (empty())
				return r.empty();
			if (r.empty())
				return false;
			return _data() == r._data();
		}
		bool operator!=(const monomial_dynamic& r) const { return !(*this == r); }

		bool operator==(const monomial_static& r) const
		{
			if (empty())
				return r.empty();
			if (r.empty())
				return false;
			return _data() == r;
		}
		bool operator!=(const monomial_static& r) const { return !(*this == r); }

		bool operator< (const monomial_static& r) const { return _mless(*this, r); }
		bool operator> (const monomial_static& r) const { return _mless(r, *this); }
		bool operator>=(const monomial_static& r) const { return !_mless(*this, r); }
		bool operator<=(const monomial_static& r) const { return !_mless(r, *this); }

		bool operator< (const monomial_dynamic& r) const { return _mless(*this, r); }
		bool operator> (const monomial_dynamic& r) const { return _mless(r, *this); }
		bool operator>=(const monomial_dynamic& r) const { return !_mless(*this, r); }
		bool operator<=(const monomial_dynamic& r) const { return !_mless(r, *this); }

		template<typename int_type>
		bool operator==(const monomial_int_t<N, D, VLess, MLess, int_type>& m) const
		{
			return m == *this;
		}
		template<typename int_type>
		bool operator!=(const monomial_int_t<N, D, VLess, MLess, int_type>& m) const
		{
			return m != *this;
		}
		template<typename int_type>
		bool operator< (const monomial_int_t<N, D, VLess, MLess, int_type>& r) const { return _mless(*this, r); }
		template<typename int_type>
		bool operator> (const monomial_int_t<N, D, VLess, MLess, int_type>& r) const { return _mless(r, *this); }
		template<typename int_type>
		bool operator>=(const monomial_int_t<N, D, VLess, MLess, int_type>& r) const { return !_mless(*this, r); }
		template<typename int_type>
		bool operator<=(const monomial_int_t<N, D, VLess, MLess, int_type>& r) const { return !_mless(r, *this); }

		unsigned degree() const { return data().degree(); }
		unsigned size()   const { return data().size(); }
		unsigned count()  const { return size(); }
		bool     empty()  const { return _ptr == nullptr; }
		
		unsigned get(varinttype var) const
		{
			if (empty())
				return 0;
			return data().get(var);
		}
		unsigned operator[](varinttype var) const
		{
			return get(var);
		}

		template<typename Mo1, typename Mo2>
		void set_multiply(const Mo1& l, const Mo2& r, bool& exponent_overflow)
		{
			if (l.empty())
				*this = r;
			else if (r.empty())
				*this = l;
			else
			{
				monomial_static tmp;
				tmp.set_multiply(l, r, exponent_overflow);
				*this = tmp;
			}
		}

		template<typename Mo1, typename Mo2>
		void set_multiply(const Mo1& l, const Mo2& r)
		{
			if (l.empty())
				*this = r;
			else if (r.empty())
				*this = l;
			else
			{
				monomial_static tmp;
				tmp.set_multiply(l, r);
				*this = tmp;
			}
		}

		template<typename Mo1, typename Mo2>
		void set_divide(const Mo1& l, const Mo2& r)
		{
			if (r.empty())
				*this = l;
			else if (l.empty())
				throw std::runtime_error("monomial_static_t::set_divide(l,r): right-hand monomial does not divide left-hand monomial!");
			else
			{
				monomial_static tmp;
				tmp.set_divide(l, r);
				*this = tmp;
			}
		}

		template<typename Mo1, typename Mo2>
		void set_lcm(const Mo1& l, const Mo2& r)
		{
			if (l.empty())
				*this = r;
			else if (r.empty())
				*this = l;
			else
			{
				monomial_static tmp;
				tmp.set_lcm(l, r);
				*this = tmp;
			}
		}

		template<typename Mo1, typename Mo2>
		void set_gcd(const Mo1& l, const Mo2& r)
		{
			if (l.empty() || r.empty())
			{
				clear();
				return;
			}
			monomial_static tmp;
			tmp.set_gcd(l, r);
			*this = tmp;
		}

		template<typename Mo>
		bool divides(const Mo& r) const
		{
			if (empty())
				return true;
			return _data().divides(r);
		}

		template<typename Mo>
		bool disjoint(const Mo& r) const
		{
			if (empty())
				return true;
			return _data().disjoint(r);
		}

		template<typename Mo>
		bool operator|(const Mo& r) const
		{
			return divides(r);
		}

		monomial_dynamic& operator*=(const monomial_static& r)
		{
			if (!r.empty())
			{
				if (empty())
					*this = r;
				else
				{
					monomial_static tmp;
					tmp.set_multiply(_data(), r);
					*this = tmp;
				}
			}
			return *this;
		}
		monomial_dynamic& operator*=(const monomial_dynamic& r)
		{
			if (!r.empty())
			{
				if (empty())
					*this = r;
				else
				{
					monomial_static tmp;
					tmp.set_multiply(_data(), r._data());
					*this = tmp;
				}
			}
			return *this;
		}
		template<typename int_type>
		monomial_dynamic& operator*=(const monomial_int_t<N, D, VLess, MLess, int_type>& r)
		{
			if (!r.empty())
			{
				if (empty())
					*this = r;
				else
				{
					monomial_static tmp;
					tmp.set_multiply(_data(), r);
					*this = tmp;
				}
			}
			return *this;
		}

		monomial_static operator*(const monomial_static& r) const
		{
			monomial_static tmp;
			tmp.set_multiply(data(), r);
			return tmp;
		}
		monomial_static operator*(const monomial_dynamic& r) const
		{
			monomial_static tmp;
			tmp.set_multiply(data(), r.data());
			return tmp;
		}
		template<typename int_type>
		monomial_static operator*(const monomial_int_t<N, D, VLess, MLess, int_type>& r) const
		{
			monomial_static tmp;
			tmp.set_multiply(data(), r);
			return tmp;
		}

		monomial_static multiply(const monomial_static& r, bool& exponent_overflow) const
		{
			monomial_static tmp;
			tmp.set_multiply(data(), r, exponent_overflow);
			return tmp;
		}
		monomial_static multiply(const monomial_dynamic& r, bool& exponent_overflow) const
		{
			monomial_static tmp;
			tmp.set_multiply(*this, r.data(), exponent_overflow);
			return tmp;
		}
		template<typename int_type>
		monomial_static multiply(const monomial_int_t<N, D, VLess, MLess, int_type>& r, bool& exponent_overflow) const
		{
			monomial_static tmp;
			tmp.set_multiply(data(), r, exponent_overflow);
			return tmp;
		}

		monomial_dynamic& operator/=(const monomial_static& r)
		{
			if (!r.empty())
			{
				if (empty())
					throw std::runtime_error("monomial_dynamic_t::operator/: empty monomial cannot be divided by non-empty monomial");
				_data() /= r;
				_data_shrink();
			}
			return *this;
		}
		monomial_dynamic& operator/=(const monomial_dynamic& r)
		{
			if (!r.empty())
			{
				if (empty())
					throw std::runtime_error("monomial_dynamic_t::operator/: empty monomial cannot be divided by non-empty monomial");
				_data() /= r._data();
				_data_shrink();
			}
			return *this;
		}
		template<typename int_type>
		monomial_dynamic& operator/=(const monomial_int_t<N, D, VLess, MLess, int_type>& r)
		{
			if (!r.empty())
			{
				if (empty())
					throw std::runtime_error("monomial_dynamic_t::operator/: empty monomial cannot be divided by non-empty monomial");
				_data() /= r;
				_data_shrink();
			}
			return *this;
		}

		monomial_static operator/(const monomial_static& r) const
		{
			monomial_static tmp;
			tmp.set_divide(data(), r);
			return tmp;
		}
		monomial_static operator/(const monomial_dynamic& r) const
		{
			monomial_static tmp;
			tmp.set_divide(data(), r.data());
			return tmp;
		}
		template<typename int_type>
		monomial_static operator/(const monomial_int_t<N, D, VLess, MLess, int_type>& r) const
		{
			monomial_static tmp;
			tmp.set_divide(data(), r);
			return tmp;
		}

		typedef const monomial_static& _static_ref_t;
		operator _static_ref_t() const
		{
			return data();
		}
		_static_ref_t data() const 
		{ 
			if (empty()) return _empty_monomial; 
			return *reinterpret_cast<const monomial_static*>(_ptr); 
		}

	private:
		char* _ptr;
		VLess _vless;
		MLess _mless;
		
		monomial_static& _data() 
		{ 
			return *reinterpret_cast<monomial_static*>(_ptr); 
		}
		const monomial_static& _data() const
		{
			return *reinterpret_cast<const monomial_static*>(_ptr);
		}

		inline void _data_malloc(std::size_t s)
		{
			_ptr = (char*)malloc(sizeof(monomial_static) - (N*sizeof(pair_t)) + (s * sizeof(pair_t)));
		}
		inline void _data_realloc(std::size_t s)
		{
			if (s == 0)
				clear();
			else
				_ptr = (char*)realloc(_ptr, sizeof(monomial_static) - (N*sizeof(pair_t)) + (s * sizeof(pair_t)));
		}
		inline void _data_shrink()
		{
			if (_ptr != nullptr)
			{
				if (_data().empty())
					clear();
				else
					_ptr = (char*)realloc(_ptr, sizeof(monomial_static) - (N*sizeof(pair_t)) + (_data().size() * sizeof(pair_t)));
			}
		}
	};
	template<std::size_t N, std::size_t D, typename VLess, typename MLess>
	typename monomial_dynamic_t<N, D, VLess, MLess>::monomial_static monomial_dynamic_t<N, D, VLess, MLess>::_empty_monomial;


	template<std::size_t N, std::size_t D, typename VL, typename ML>
	unsigned degree(const monomial_static_t<N, D, VL, ML>& m)
	{
		return m.degree();
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	unsigned degree(const monomial_dynamic_t<N, D, VL, ML>& m)
	{
		return m.degree();
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	unsigned degree(const monomial_int_t<N, D, VL, ML, IT>& m)
	{
		return m.degree();
	}

	template<std::size_t N, std::size_t D, typename VL, typename ML>
	unsigned count(const monomial_static_t<N, D, VL, ML>& m)
	{
		return m.count();
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	unsigned count(const monomial_dynamic_t<N, D, VL, ML>& m)
	{
		return m.count();
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	unsigned count(const monomial_int_t<N, D, VL, ML, IT>& m)
	{
		return m.count();
	}

	template<std::size_t N, std::size_t D, typename VL, typename ML>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_static_t<N, D, VL, ML>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l, r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l.data(), r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_static_t<N, D, VL, ML>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l, r.data());
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l.data(), r.data());
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_static_t<N, D, VL, ML>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l, r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l.data(), r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l, r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l, r.data());
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> lcm(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_lcm(l, r);
		return tmp;
	}


	template<std::size_t N, std::size_t D, typename VL, typename ML>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_static_t<N, D, VL, ML>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l, r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l.data(), r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_static_t<N, D, VL, ML>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l, r.data());
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l.data(), r.data());
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_static_t<N, D, VL, ML>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l, r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l.data(), r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l, r);
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l, r.data());
		return tmp;
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	monomial_static_t<N, D, VL, ML> gcd(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		monomial_static_t<N, D, VL, ML> tmp;
		tmp.set_gcd(l, r);
		return tmp;
	}

	template<std::size_t N, std::size_t D, typename VL, typename ML>
	bool disjoint(const monomial_static_t<N, D, VL, ML>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		return l.disjoint(r);
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	bool disjoint(const monomial_static_t<N, D, VL, ML>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		return l.disjoint(r.data());
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	bool disjoint(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		return l.data().disjoint(r);
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML>
	bool disjoint(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		return l.data().disjoint(r.data());
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	bool disjoint(const monomial_static_t<N, D, VL, ML>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		return l.disjoint(r);
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	bool disjoint(const monomial_dynamic_t<N, D, VL, ML>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		return l.data().disjoint(r);
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	bool disjoint(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_static_t<N, D, VL, ML>& r)
	{
		return l.disjoint(r);
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	bool disjoint(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_dynamic_t<N, D, VL, ML>& r)
	{
		return l.disjoint(r.data());
	}
	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	bool disjoint(const monomial_int_t<N, D, VL, ML, IT>& l, const monomial_int_t<N, D, VL, ML, IT>& r)
	{
		return l.disjoint(r);
	}

} //namespace gb

namespace std
{

	template<std::size_t N, std::size_t D, typename VLess, typename MLess>
	struct hash<gb::monomial_static_t<N, D, VLess, MLess> >
	{
		template<typename Mo>
		std::size_t operator()(const Mo& m) const
		{
			using gb::detail::hash_combine;
			std::size_t seed = m.count();
			for (auto ve : m)
			{
				hash_combine(seed, ve.first);
				hash_combine(seed, ve.second);
			}
			return seed;
		}
	};

	template<std::size_t N, std::size_t D, typename VLess, typename MLess>
	struct hash<gb::monomial_dynamic_t<N, D, VLess, MLess> >
	{
		template<typename Mo>
		std::size_t operator()(const Mo& m) const
		{
			using gb::detail::hash_combine;
			std::size_t seed = m.count();
			for (auto ve : m)
			{
				hash_combine(seed, ve.first);
				hash_combine(seed, ve.second);
			}
			return seed;
		}
	};

	template<std::size_t N, std::size_t D, typename VLess, typename MLess, typename Int>
	struct hash<gb::monomial_int_t<N, D, VLess, MLess, Int> >
	{
		typedef gb::monomial_int_t<N, D, VLess, MLess, Int> monomial_int;
		typedef typename monomial_int::int_type int_type;

		hash<int_type> _subhasher;

		std::size_t operator()(const monomial_int& m) const
		{
			return _subhasher(m.value());
		}

		template<typename Mo>
		std::size_t operator()(const Mo& m) const
		{
			using gb::detail::hash_combine;
			std::size_t seed = m.count();
			for (auto ve : m)
			{
				hash_combine(seed, ve.first);
				hash_combine(seed, ve.second);
			}
			return seed;
		}
	};

	template<std::size_t N, std::size_t D, typename ML>
	std::ostream& operator<<(std::ostream& o, const typename gb::monomial_static_t<N, D, typename gb::monomial_traits<N, D>::var_less, ML>& monomial)
	{
		if (monomial.empty())
			return o << "1";
		for (auto it = monomial.begin(); it != monomial.end(); ++it)
		{
			if (it != monomial.begin())
				o << "*X";
			else
				o << "X";
			o << (unsigned)(it->first);
			if (it->second > 1)
				o << "^" << (unsigned)(it->second);
		}
		return o;
	}

	template<std::size_t N, std::size_t D, typename ML>
	std::ostream& operator<<(std::ostream& o, const typename gb::monomial_static_t<N, D, typename gb::monomial_traits<N, D>::var_greater, ML>& monomial)
	{
		if (monomial.empty())
			return o << "1";
		for (auto it = monomial.rbegin(); it != monomial.rend(); ++it)
		{
			if (it != monomial.rbegin())
				o << "*X";
			else
				o << "X";
			o << (unsigned)(it->first);
			if (it->second > 1)
				o << "^" << (unsigned)(it->second);
		}
		return o;
	}

	template<std::size_t N, std::size_t D, typename ML>
	std::ostream& operator<<(std::ostream& o, const typename gb::monomial_dynamic_t<N, D, typename gb::monomial_traits<N, D>::var_less, ML>& monomial)
	{
		if (monomial.empty())
			return o << "1";
		for (auto it = monomial.begin(); it != monomial.end(); ++it)
		{
			if (it != monomial.begin())
				o << "*X";
			else
				o << "X";
			o << (unsigned)(it->first);
			if (it->second > 1)
				o << "^" << (unsigned)(it->second);
		}
		return o;
	}

	template<std::size_t N, std::size_t D, typename ML>
	std::ostream& operator<<(std::ostream& o, const typename gb::monomial_dynamic_t<N, D, typename gb::monomial_traits<N, D>::var_greater, ML>& monomial)
	{
		if (monomial.empty())
			return o << "1";
		for (auto it = monomial.rbegin(); it != monomial.rend(); ++it)
		{
			if (it != monomial.rbegin())
				o << "*X";
			else
				o << "X";
			o << (unsigned)(it->first);
			if (it->second > 1)
				o << "^" << (unsigned)(it->second);
		}
		return o;
	}

	template<std::size_t N, std::size_t D, typename VL, typename ML, typename IT>
	std::ostream& operator<<(std::ostream& o, const typename gb::monomial_int_t<N, D, VL, ML, IT>& monomial)
	{
		typename gb::monomial_static_t<N, D, VL, ML> tmp(monomial);
		return o << tmp;
	}

} // namespace gb

#endif // M4GB_MONOMIAL_BASE_HPP
