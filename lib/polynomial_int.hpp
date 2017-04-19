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

#ifndef M4GB_POLYNOMIAL_INT_HPP
#define M4GB_POLYNOMIAL_INT_HPP

#include "detail.hpp"
#include "polynomial_simple.hpp"

#include <vector>
#include <map>
#include <iostream>
#include <array>


namespace gb
{

	template<typename Traits, typename GF>
	class polynomial_int_t
	{
	public:
		typedef Traits                                   traits_t;
		typedef GF                                       field_t;
		
		typedef typename GF::gfelm_t                     coefficient_t;

		typedef typename traits_t::dynamic_monomial_t    dynamic_monomial_t;
		typedef typename traits_t::static_monomial_t     static_monomial_t;
		typedef typename traits_t::int_monomial_t        int_monomial_t;

		typedef int_monomial_t monomial_t;
		typedef std::pair<coefficient_t, int_monomial_t> term_t;
		typedef typename int_monomial_t::order_tag_t     order_tag_t;

		static const std::size_t max_vars = traits_t::max_vars;
		static const std::size_t max_deg = traits_t::max_deg;
		static const std::size_t fieldsize = GF::gfsize;
		static const std::size_t fieldchar = GF::gfchar;

		// not supported by g++-4.8
		//static_assert(std::is_trivially_copyable<coefficient_t>::value, "coefficient_t is not trivially copyable!");

		static field_t field;

		typedef std::pair<coefficient_t, static_monomial_t>   static_term_t;
		typedef std::pair<coefficient_t, dynamic_monomial_t>  dynamic_term_t;

		typedef typename std::vector<term_t>::const_iterator          const_iterator;
		typedef typename std::vector<term_t>::const_reverse_iterator  const_reverse_iterator;

	private:
		typedef typename std::vector<term_t>::iterator          _iterator;
		typedef typename std::vector<term_t>::reverse_iterator  _reverse_iterator;
		const_iterator          _begin() const { return _terms.begin(); }
		const_iterator          _end()   const { return _terms.end(); }
		_iterator               _begin()       { return _terms.begin(); }
		_iterator               _end()         { return _terms.end(); }

		std::vector<term_t>  _terms;
		struct _less_term_t {
			bool operator()(const int_monomial_t& l, const int_monomial_t& r) const { return l < r; }
			bool operator()(const int_monomial_t& l, const term_t& r) const { return l < r.second; }
			bool operator()(const term_t& l, const int_monomial_t& r) const { return l.second < r; }
			bool operator()(const term_t& l, const term_t& r) const { return l.second < r.second; }
		} _less_term;

	public:

		polynomial_int_t()
		{
		}

		polynomial_int_t(const polynomial_int_t& r)
		{
			_terms = r._terms;
		}

		polynomial_int_t(polynomial_int_t&& r)
		{
			std::swap(_terms, r._terms);
		}

		template<typename Iter>
		explicit polynomial_int_t(Iter first, Iter last)
		{
			assign(first, last);
		}

		polynomial_int_t& operator=(const polynomial_int_t& r)
		{
			_terms = r._terms;
			return *this;
		}
		polynomial_int_t& operator=(polynomial_int_t&& r)
		{
			std::swap(_terms, r._terms);
			return *this;
		}

		void swap(polynomial_int_t& r)
		{
			std::swap(_terms, r._terms);
		}


		template<typename Container>
		polynomial_int_t& operator=(Container& r)
		{
			assign(r.begin(), r.end());
			return *this;
		}

		template<typename Container>
		void assign(Container& r)
		{
			assign(r.begin(), r.end());
		}

		template<typename Iter>
		void assign(Iter first, Iter last)
		{
			clear();

			std::size_t count = std::distance(first, last);
			_terms.reserve(count);

			for (; first != last; ++first)
			{
				if (first->first == 0)
					continue;
				_terms.emplace_back(*first);
			}

			std::sort(_terms.begin(), _terms.end(), _less_term);

			for (std::size_t i = 0; i < _terms.size();)
			{
				std::size_t j = i + 1;
				for (; j < _terms.size() && _terms[j].second == _terms[i].second; ++j)
					_terms[i].first += _terms[j].first;
				if (j != i + 1)
				{
					if (_terms[i].first == 0)
					{
						_terms.erase(_terms.begin() + i, _terms.begin() + j);
					}
					else
					{
						_terms.erase(_terms.begin() + i + 1, _terms.begin() + j);
						++i;
					}
				}
				else
				{
					++i;
				}
			}

			_terms.shrink_to_fit();

			test();
		}

		void clear()
		{
			_terms.clear();
		}

		int force_test()
		{
			for (auto it = begin(); it != end(); ++it)
				if (it->first == 0)
					throw std::runtime_error("polynomial_simple_t::test(): zero coefficient found in terms!");

			if (!empty())
				for (auto it = begin(), it2 = it++; it != end(); ++it, ++it2)
					if (!(it2->second < it->second))
						throw std::runtime_error("polynomial_simple_t::test(): terms not in correct order!");
			return 0;
		}

		void test()
		{
#ifdef POLYNOMIAL_TEST
			force_test();
#endif // POLYNOMIAL_TEST
		}

		bool operator==(const polynomial_int_t& r) const
		{
			return _terms == r._terms;
		}

		bool operator!=(const polynomial_int_t& r) const
		{
			return _terms != r._terms;
		}


		bool empty() const
		{
			return _terms.empty();
		}

		std::size_t count() const
		{
			return _terms.size();
		}

		std::size_t size() const
		{
			return _terms.size();
		}

		unsigned degree() const
		{
			if (empty())
				return 0;
			return leading_monomial().degree();
		}


		const_iterator          begin() const { return _terms.begin(); }
		const_iterator          end()   const { return _terms.end(); }
		const_reverse_iterator rbegin() const { return _terms.rbegin(); }
		const_reverse_iterator rend()   const { return _terms.rend(); }

		const_iterator          begin_smallest() const { return _terms.begin(); }
		const_iterator          end_smallest()   const { return _terms.end(); }
		const_reverse_iterator  begin_largest()  const { return _terms.rbegin(); }
		const_reverse_iterator  end_largest()    const { return _terms.rend(); }

		polynomial_int_t& operator*=(coefficient_t c)
		{
			if (c == 0)
				clear();
			else if (c != 1)
			{
				for (auto it = _terms.begin(); it != _terms.end(); ++it)
					it->first = mul_nonzero(it->first, c);

				test();
			}
			return *this;
		}

		polynomial_int_t operator*(coefficient_t c) const
		{
			polynomial_int_t tmp;
			if (c == 1)
				tmp = *this;
			else
			{
				if (c != 0 && _terms.size() != 0)
				{
					tmp._terms.reserve(_terms.size());
					for (auto term : _terms)
						tmp._terms.emplace_back(mul_nonzero(term.first, c), term.second);
				}
			}
			tmp.test();
			return tmp;
		}

		polynomial_int_t& operator*=(const static_monomial_t& m)
		{
			if (!m.empty())
			{
				bool exponent_has_decreased = false;
				for (auto it = _terms.begin(); it != _terms.end(); ++it)
				{
					it->second = it->second.multiply(m, exponent_has_decreased);
					if (exponent_has_decreased)
					{
						exponent_has_decreased = false;
						auto it2 = std::lower_bound(_terms.begin(), it, *it, _less_term);
						if (it2 != it)
						{
							if (it2->second == it->second)
							{
								it2->first += it->first;
								it = _terms.erase(it);
								--it;
								if (it2->first == 0)
								{
									--it;
									_terms.erase(it2);
								}
							}
							else
							{
								term_t tmp = std::move(*it);
								std::move_backward(it2, it, it + 1);
								*it2 = std::move(tmp);
							}
						}
					}
				}
				test();
			}
			return *this;
		}
		polynomial_int_t& operator*=(const dynamic_monomial_t& m)
		{
			return *this *= m.data();
		}
		polynomial_int_t& operator*=(const int_monomial_t& m)
		{
			static_monomial_t tmp(m);
			return *this *= tmp;
		}

		polynomial_int_t operator*(const static_monomial_t& m) const
		{
			polynomial_int_t tmp;
			if (empty() || m.empty())
				tmp = *this;
			else
			{
				tmp._terms.reserve(_terms.size());
				bool exponent_has_decreased = false;
				for (auto term : _terms)
				{
					tmp._terms.emplace_back(term.first, term.second.multiply(m, exponent_has_decreased));
					if (exponent_has_decreased)
					{
						exponent_has_decreased = false;
						auto it = tmp._terms.end() - 1;
						auto it2 = std::lower_bound(tmp._terms.begin(), it, *it, _less_term);
						if (it2 != it)
						{
							if (it2->second == it->second)
							{
								it2->first += it->first;
								tmp._terms.pop_back();
								if (it2->first == 0)
									tmp._terms.erase(it2);
							}
							else
							{
								term_t tmp = std::move(*it);
								std::move_backward(it2, it, it + 1);
								*it2 = std::move(tmp);
							}
						}
					}
				}
			}
			tmp.test();
			return tmp;
		}
		polynomial_int_t operator*(const dynamic_monomial_t& m) const
		{
			return *this * m.data();
		}
		polynomial_int_t operator*(const int_monomial_t& m) const
		{
			static_monomial_t tmp(m);
			return *this * tmp;
		}

		polynomial_int_t& multiply_this(const coefficient_t& c, const static_monomial_t& m)
		{
			if (c == 0)
				clear();
			else if (m.count() == 0)
				*this *= c;
			else if (c == 1)
				*this *= m;
			else
			{
				bool exponent_has_decreased = false;
				for (auto it = _terms.begin(); it != _terms.end(); ++it)
				{
					it->first = mul_nonzero(it->first, c);
					it->second = it->second.multiply(m, exponent_has_decreased);
					if (exponent_has_decreased)
					{
						exponent_has_decreased = false;
						auto it2 = std::lower_bound(_terms.begin(), it, *it, _less_term);
						if (it2 != it)
						{
							if (it2->second == it->second)
							{
								it2->first += it->first;
								it = _terms.erase(it);
								--it;
								if (it2->first == 0)
								{
									--it;
									_terms.erase(it2);
								}
							}
							else
							{
								term_t tmp = std::move(*it);
								std::move_backward(it2, it, it + 1);
								*it2 = std::move(tmp);
							}
						}
					}
				}
			}
			test();
			return *this;
		}
		polynomial_int_t& operator*=(const term_t& t)
		{
			static_monomial_t tmp(t.second);
			return multiply_this(t.first, tmp);
		}
		polynomial_int_t& operator*=(const static_term_t& t)
		{
			return multiply_this(t.first, t.second);
		}
		polynomial_int_t& operator*=(const dynamic_term_t& t)
		{
			return multiply_this(t.first, t.second.data());
		}

		polynomial_int_t multiply(const coefficient_t& c, const static_monomial_t& m) const
		{
			polynomial_int_t tmp;
			if (c == 0)
			{
			}
			else if (m.count() == 0)
				tmp = (*this) * c;
			else if (c == 1)
				tmp = (*this) * m;
			else
			{
				tmp._terms.reserve(_terms.size());
				bool exponent_has_decreased = false;
				for (auto term : _terms)
				{
					tmp._terms.emplace_back(mul_nonzero(term.first, c), term.second.multiply(m, exponent_has_decreased));
					if (exponent_has_decreased)
					{
						exponent_has_decreased = false;
						auto it = tmp._terms.end() - 1;
						auto it2 = std::lower_bound(tmp._terms.begin(), it, *it, _less_term);
						if (it2 != it)
						{
							if (it2->second == it->second)
							{
								it2->first += it->first;
								tmp._terms.pop_back();
								if (it2->first == 0)
									tmp._terms.erase(it2);
							}
							else
							{
								term_t tmp = std::move(*it);
								std::move_backward(it2, it, it + 1);
								*it2 = std::move(tmp);
							}
						}
					}
				}
			}
			tmp.test();
			return tmp;
		}
		polynomial_int_t operator*(const term_t& t) const
		{
			return multiply(t.first, static_monomial_t(t.second));
		}
		polynomial_int_t operator*(const static_term_t& t) const
		{
			return multiply(t.first, t.second);
		}
		polynomial_int_t operator*(const dynamic_term_t& t) const
		{
			return multiply(t.first, t.second.data());
		}

		polynomial_int_t operator+(const polynomial_int_t& r) const
		{
			polynomial_int_t tmp;
			if (r.empty())
				tmp = *this;
			else if (empty())
				tmp = r;
			else
			{
				tmp._terms.reserve(count() + tmp.count());
				auto itl = _terms.begin(), itle = _terms.end();
				auto itr = r._terms.begin(), itre = r._terms.end();
				while (true)
				{
					if (itl->second == itr->second)
					{
						coefficient_t s = itl->first + itr->first;
						if (s != 0)
							tmp._terms.emplace_back(s, itl->second);
						++itl; ++itr;
						if (itl == itle || itr == itre)
							break;
					}
					else if (itl->second < itr->second)
					{
						tmp._terms.emplace_back(*itl);
						if (++itl == itle)
							break;
					}
					else
					{
						tmp._terms.emplace_back(*itr);
						if (++itr == itre)
							break;
					}
				}
				for (; itl != itle; ++itl)
					tmp._terms.emplace_back(*itl);
				for (; itr != itre; ++itr)
					tmp._terms.emplace_back(*itr);
				tmp._terms.shrink_to_fit();
			}
			tmp.test();
			return tmp;
		}

		polynomial_int_t operator-(const polynomial_int_t& r) const
		{
			polynomial_int_t tmp;
			if (r.empty())
				tmp = *this;
			else if (empty())
			{
				tmp = r;
				for (auto t : tmp._terms)
					t.first = -t.first;
			}
			else
			{
				tmp._terms.reserve(count() + tmp.count());
				auto itl = _terms.begin(), itle = _terms.end();
				auto itr = r._terms.begin(), itre = r._terms.end();
				while (true)
				{
					if (itl->second == itr->second)
					{
						coefficient_t s = itl->first - itr->first;
						if (s != 0)
							tmp._terms.emplace_back(s, itl->second);
						++itl; ++itr;
						if (itl == itle || itr == itre)
							break;
					}
					else if (itl->second < itr->second)
					{
						tmp._terms.emplace_back(*itl);
						if (++itl == itle)
							break;
					}
					else
					{
						tmp._terms.emplace_back(-itr->first, itr->second);
						if (++itr == itre)
							break;
					}
				}
				for (; itl != itle; ++itl)
					tmp._terms.emplace_back(*itl);
				for (; itr != itre; ++itr)
					tmp._terms.emplace_back(-itr->first, itr->second);
				tmp._terms.shrink_to_fit();
			}
			tmp.test();
			return tmp;
		}

		void reserve(std::size_t size)
		{
			_terms.reserve(size);
		}
		polynomial_int_t& operator+=(const term_t& t)
		{
			if (leading_monomial() < t.second)
			{
				_terms.emplace_back(t);
			}
			else
			{
				auto it = std::lower_bound(_begin(), _end(), t, _less_term);
				if (it->second == t.second)
				{
					it->first += t.first;
					if (it->first == 0)
						_terms.erase(it);
				}
				else
				{
					_terms.emplace(it, t);
				}
			}
			return *this;
		}
		void add_head(const term_t& t)
		{
			_terms.emplace_back(t);
		}
		void add_head(const coefficient_t& c, const int_monomial_t& m)
		{
			_terms.emplace_back(c, m);
		}
		void pop_head()
		{
			_terms.pop_back();
		}
		const_iterator erase(const_iterator it)
		{
			return _terms.erase(it);
		}

		const term_t&          leading_term()        const { return _terms.back(); }
		const int_monomial_t&  leading_monomial()    const { return _terms.back().second; }
		const coefficient_t&   leading_coefficient() const { return _terms.back().first; }


		static void print(std::ostream& o, const term_t& t)
		{
			if (t.first == 0)
				return;
			if (t.first > 1)
				o << t.first << "*";
			o << t.second;
		}
	};

	template<typename Traits, typename GF>
	unsigned degree(const polynomial_int_t<Traits, GF>& p)
	{
		return p.degree();
	}

	template<typename Traits, typename GF>
	std::size_t count(const polynomial_int_t<Traits, GF>& p)
	{
		return p.count();
	}

} //namespace gb

namespace std
{
	template<typename Traits, typename GF>
	std::ostream& operator<<(std::ostream& o, const gb::polynomial_int_t<Traits, GF>& poly)
	{
		bool first = true;

		for (auto it = poly.begin_largest(); it != poly.end_largest(); ++it)
		{
			if (first) first = false; else o << " + ";
			poly.print(o, *it);
		}
		if (poly.empty())
			o << "0";
		return o;
	}
}

#endif // M4GB_POLYNOMIAL_INT_HPP
