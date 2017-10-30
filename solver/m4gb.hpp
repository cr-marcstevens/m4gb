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

#ifndef M4GB_SOLVER_M4GB_HPP
#define M4GB_SOLVER_M4GB_HPP

#include <map>
#include <set>
#include <vector>
#include <deque>
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <algorithm>

#include "config.hpp"
#include "../lib/gf_elem_simple.hpp"
#include "../lib/monomial_degrevlex.hpp"
#include "../lib/polynomial_int.hpp"
#include "../lib/solver_base.hpp"

////// LIMITS
//#define USE_CP_SIMPLIFY

///// SET VARIATIONS
#define USETHREADS // NOT FULLY IMPLEMENTED

// rowreduce input system before passing through update
#define PREROWREDUCE

// postprocessing: before update interreduce to add with current basis to obtain minimal toadd set
// (essentially shortcuts the 'basis collapse' event)
#define ROWREDUCE_POSTPROCESSING

// postprocessing: only leadreduce polys while postprocessing, todo: final full reduce (will now happen during update/shrink cycle)
#define POSTPROCESSING_ONLYLEADREDUCE // seems slightly faster

#define BASISSIEVE

// maximum number of critical pairs to handle in 1 iteration, undefined == infinity
#define MAXSELECTION 512

// experimental: try using largest available basis multiple in polymatrix to compute matrixrow, instead of basispoly itself
// reasoning: might lead to fewer reducable monomials during multiplication => lower complexity
//#define SELECTION_USE_LARGEST_AVAILABLE_BASIS_MULTIPLE // default:off

// true = compute basis multiples on demand, false = compute all when inserting basis element or increasing upperbound
#define LAZY_COMPUTATIONS true

// when defined: on matrix shrink: set new generation for dense_index, only update older matrix polys to latest generation just-in-time
#define LAZY_SHRINK

// when LM(f) | LM(g) immediate add spoly(g,f) in to_add queue
#define IMMEDIATE_BASIS_REDUCE

// when matrix row has not been evaluated yet, opportunistically replace it with other basis multiple computed for critical pair
// without MATRIXSWAP our F4 matrix is actually larger (more rows) and for each CP-lcm includes a multiple of a particular choice of a basis poly
// this is more work, but can also find more new basis elements early with relatively little additional work
//#define MATRIXSWAP  // disabled by default: little effect on sparse systems, makes solving dense mq challenges somewhat more expensive

// only set basis multiple in basisitem when it is actually computed
//#define LAZY_MATRIXMULT


// !BREAKING! VARIATIONS: THESE OPTIONS MAY CAUSE ENDRESULT NOT BEING GROEBNER BASIS

// by observation for solving MQ challenges: 
//  1. lcmdegree increases until it collapses
//  2. once a batch returns zero toadd polys
//  3. all remaining batches of same lcmdegree return zero toadd polys
// this option essentially skips all remaining batches of same lcmdegree
//#define TWEAK_SKIP_SAMEDEGREELCM_AFTER_ZEROTOADD


///// SET DEBUGGING
//#define MQDEBUG
//#define MQDEBUGTESTPOLY

#ifdef MQDEBUG
#define MQDEBUGCHECK(s) if (s) throw;
#ifdef MQDEBUGTESTPOLY
#define MQDEBUGPOLY(s) test_poly(get_polynomial(s));
#else
#define MQDEBUGPOLY(s)
#endif
#else
#define MQDEBUGCHECK(s)
#define MQDEBUGPOLY(s)
#endif

#ifdef USETHREADS
#include <atomic>
#include <mutex>
#include <thread>
#include <boost/thread.hpp>
#include "../contrib/threadpool.hpp"
#endif

namespace gb
{

	typedef polynomial_int_t<monomial_degrevlex_traits_int<MAXVARS, FIELDSIZE, INT_MONOMIAL_SIZE>, myfield_t> myintpolynomial_t;

	template<typename Container, typename Iterator>
	void container_extract(Container& c, Iterator& it, typename std::remove_const< typename std::add_lvalue_reference<typename Iterator::value_type>::type >::type v)
	{
#ifdef MQDEBUG
		v = *it;
#else
		v = std::move(const_cast<typename std::remove_const< typename std::add_lvalue_reference<typename Iterator::value_type>::type >::type>(*it));
#endif
		it = c.erase(it);
	}

	template<typename poly_t>
	class polymatrix
	{
	public:
		typedef poly_t polynomial_t;                                             // generic polynomial
		typedef typename polynomial_t::term_t                term_t;             // term type: pair: (coefficient, monomial)
		typedef typename polynomial_t::coefficient_t         coefficient_t;      // finite field coefficient type
		typedef typename polynomial_t::int_monomial_t        int_monomial_t;     // integer monomial representation
		typedef typename polynomial_t::static_monomial_t     static_monomial_t;  // static size natural monomial representation
		typedef typename polynomial_t::dynamic_monomial_t    dynamic_monomial_t; // dynamic size natural monomial representation


		// maximum number of variables supported by current monomial&polynomial types
		static const std::size_t max_vars = polynomial_t::max_vars;

		// dense representation for all tails of polynomials: vector of coefficients for (non-reducable) monomials given in polymatrix's global dense_index
		typedef std::vector<coefficient_t> dense_poly_t;

		// polymatrix entry: multiple of basis polynomial (given by LM blm), will be computed just-in-time
		// Note: just stores the tail of the polynomial, the LM is the index of the polymatrix entry and stored separately
		struct lazy_entry_t
		{
			dense_poly_t tail;
#ifdef USETHREADS
			std::atomic_int_fast32_t generation;
			std::mutex mutex;
#else
			int generation;
#endif
			int_monomial_t blm;

			lazy_entry_t()
				: generation(-1)
			{}
			lazy_entry_t(const int_monomial_t& _blm)
				: generation(-1), blm(_blm)
			{}
			lazy_entry_t(const dense_poly_t& _tail, int _generation, const int_monomial_t& _blm)
				: tail(_tail), generation(_generation), blm(_blm)
			{}
			lazy_entry_t(dense_poly_t&& _tail, int _generation, const int_monomial_t& _blm)
				: tail(std::move(_tail)), generation(_generation), blm(_blm)
			{}
			lazy_entry_t(const lazy_entry_t& r)
				: tail(r.tail), generation(r.generation), blm(r.blm)
			{}
			lazy_entry_t(lazy_entry_t&& r)
				: tail(std::move(r.tail)), generation((const int)(r.generation)), blm(r.blm)
			{}
		};

		// the type of the actual polymatrix: a database of multiples of basis polynomials: stored as (hash)map: LM => tail
		typedef std::unordered_map<int_monomial_t, lazy_entry_t> matrix_type;

		// type that stores both integer monomial and its degree: used for global vector of (non-reducable) monomials for dense poly rep.
		struct monorep_t
		{
			int_monomial_t intm;
			unsigned degm;
			monorep_t()
			{
				MQDEBUGCHECK(true);
			}

			monorep_t(const int_monomial_t& m)
				:intm(m), degm(intm.degree())
			{
			}
			monorep_t(const static_monomial_t& m)
				:intm(m), degm(intm.degree())
			{
			}
			monorep_t(const dynamic_monomial_t& m)
				:intm(m), degm(intm.degree())
			{
			}

			bool operator<(const int_monomial_t& m) const
			{
				return intm < m;
			}
			bool operator<(const unsigned& d) const
			{
				return degm < d;
			}
		};

		// information stored for each basis poly:
		// - LM of polymatrix multiples 'belonging' to this basis poly
		// - mul,res: some information used to determine basis LM divisors of new monomials when increasing upperbound
		struct basisrep_t
		{
			std::unordered_set<int_monomial_t> matrix_multiples;
#ifdef BASISSIEVE
			int_monomial_t mul;
			int_monomial_t res;
#endif

			basisrep_t& assign(const int_monomial_t& _lm)
			{
				matrix_multiples.clear();
				matrix_multiples.insert(_lm);
#ifdef BASISSIEVE
				mul = 1;
				res = _lm * mul;
#endif
				return *this;
			}
		};

		// the global monomial upper bound: every monomial in the polymatrix is smaller than this upper bound
		int_monomial_t upper_bound;
		// the global vector of (non-reducable) monomials for dense representation
		std::vector<monorep_t> dense_index;
		// a copy to keep track of remaining non-reducable monomials during GB-update
		std::set<int_monomial_t> dense_index2;
		// inverse mapping: (non-reducable) monomial => offset in dense_index
		std::unordered_map<int_monomial_t, std::size_t> dense_invindex;

		// a copy for each previous generation
		std::vector< std::vector<int_monomial_t> > dense_index_generation;
		int generation;

		// the polymatrix of multiples of basis polies: LM => tail, note LM = reducable monomial, tail consists of non-reducable monomials
		matrix_type matrix;

		// the ordered index of the basis: LM => deg(LM), the actual tail is stored inside the polymatrix
		std::map<int_monomial_t, unsigned> basis;

		// some more information about each basis poly
		std::unordered_map<int_monomial_t, basisrep_t> basisitem;

#ifdef USETHREADS
		// threadpool for polymatrix operations
		typedef typename ::ctpl::thread_pool                 thread_pool_t;
		thread_pool_t threadpool;
		std::mutex mut;
		typedef std::lock_guard<std::mutex> lock_type;
#endif


		polymatrix()
			: upper_bound(0), generation(0)
#ifdef USETHREADS
			, threadpool(0, (1 << 16))
#endif
		{
		}

		void clear()
		{
			basis.clear();
			basisitem.clear();
			matrix.clear();
			dense_index.clear();
			dense_index2.clear();
			dense_invindex.clear();
			dense_index_generation.clear();
			upper_bound = 0;
			generation = 0;
		}

		// ordering of polynomial by ordering of LM: for dense representation that means ordering by length of coefficient vector (no trailing zero coefficients allowed)
		struct polyrep_less_t
		{
			bool operator()(const dense_poly_t& l, const dense_poly_t& r) const
			{
				return l.size() < r.size();
			}
		};
		polyrep_less_t get_polyrep_less() const { return polyrep_less_t(); }

		// iterators and look-up in polymatrix
		typedef typename matrix_type::iterator       iterator;
		typedef typename matrix_type::const_iterator const_iterator;
		iterator begin()       { return matrix.begin(); }
		iterator end()         { return matrix.end(); }
		const_iterator begin() const { return matrix.begin(); }
		const_iterator end()   const { return matrix.end(); }
		iterator       find(const int_monomial_t& m) { return matrix.find(m); }
		const_iterator find(const int_monomial_t& m) const { return matrix.find(m); }
		std::size_t    size() const  { return matrix.size(); }

		// insert placeholder for poly u * f with f in basis, LM(f)=blm and u=lm/blm in polymatrix
		void insert(const int_monomial_t& lm, const int_monomial_t& blm)
		{
			auto itb = matrix.emplace(lm, blm);
			if (itb.second == false)
				throw std::runtime_error("matrix::insert(): value already present!");
		}

		// insert poly in polymatrix: LM explicit, tail in dense representation
		template<typename Tail>
		void insert(const int_monomial_t& lm, Tail&& tail, const int_monomial_t& glm)
		{
			MQDEBUGCHECK(tail.size() > dense_index.size() || (!tail.empty() && tail.back() == 0));

			auto itb = matrix.emplace(lm, lazy_entry_t(std::forward<Tail>(tail), generation, glm));
			if (itb.second == false)
				throw std::runtime_error("matrix::insert(): value already present!");
		}

		// insert placeholder for poly u * f, with f in basis, LM(f) = glm, uglm = u * glm, br = basisitem[glm]
		void create_basis_multiple(const int_monomial_t& glm, const static_monomial_t& u, const int_monomial_t& uglm, basisrep_t& br, bool lazy = false)
		{
			if (lazy)
				insert(uglm, glm);
			else
				insert(uglm, get_u_g(u, glm), glm);

			MQDEBUGPOLY(uglm);
#ifndef LAZY_MATRIXMULT
			br.matrix_multiples.insert(uglm);
#endif
		}

		// decrease upper_bound: erase all polynomials LM > m and all non-reducable monomials M > m
		void decrease_upper_bound(int_monomial_t m)
		{
			if (m >= upper_bound)
				return;
			upper_bound = int_monomial_t(m.value() + 1);
			auto it = std::lower_bound(dense_index.begin(), dense_index.end(), upper_bound);
			for (auto it2 = it; it2 != dense_index.end(); ++it2)
			{
				dense_invindex.erase(it2->intm);
				dense_index2.erase(it2->intm);
			}
			dense_index.erase(it, dense_index.end());
			for (auto it2 = matrix.begin(); it2 != matrix.end();)
				if (it2->first >= upper_bound)
					it2 = matrix.erase(it2);
				else
					++it2;
#ifdef BASISSIEVE
			for (auto& bi : basisitem)
			{
				bi.second.mul = int_monomial_t(upper_bound.degree() - bi.first.degree(), minimum_of_degree_tag());
				bi.second.res = bi.first * bi.second.mul;
			}
#endif
		}

		// increase upperbound: ensure that upper_bound > m, never decreases upperbound
		// for every new monomial check divisability by basis LM: => create basis multiple in polymatrix, otherwise add to vector of non-reducable monomials
		void increase_upper_bound(int_monomial_t m)
		{
#ifdef BASISSIEVE
			if (m < upper_bound)
				return;
			std::vector< std::pair<int_monomial_t, int_monomial_t> > divisor(m.value() - upper_bound.value() + 1);
			static_monomial_t BLM;
			for (auto b = basis.rbegin(); b != basis.rend(); ++b)
			{
				auto& bi = *basisitem.find(b->first);
				BLM = bi.first;
#ifdef MQDEBUG
				if (bi.first * int_monomial_t(bi.second.mul.value() - 1) >= upper_bound)
					throw;
#endif
				while (bi.second.res < upper_bound)
				{
					bi.second.mul = int_monomial_t(bi.second.mul.value() + 1);
					bi.second.res = bi.second.mul * BLM;
				}
				while (bi.second.res <= m)
				{
					divisor[bi.second.res.value() - upper_bound.value()].first = bi.first;
					divisor[bi.second.res.value() - upper_bound.value()].second = bi.second.mul;
					bi.second.mul = int_monomial_t(bi.second.mul.value() + 1);
					bi.second.res = bi.second.mul * BLM;
				}
			}
			for (std::size_t i = 0; i < divisor.size(); ++i)
			{
				if (divisor[i].first == 0)
				{
					dense_invindex[upper_bound] = dense_index.size();
					dense_index.emplace_back(upper_bound);
					dense_index2.emplace(upper_bound);
				}
				else
				{
					create_basis_multiple(divisor[i].first, divisor[i].second, upper_bound, basisitem[divisor[i].first], LAZY_COMPUTATIONS);
				}
				upper_bound = int_monomial_t(upper_bound.value() + 1);
			}
#else
			static_monomial_t UB;
			for (; upper_bound <= m; upper_bound = int_monomial_t(upper_bound.value() + 1))
			{
				UB = upper_bound;
				const unsigned ubdeg = UB.degree();
				bool isreducable = false;
				for (auto bit = basis.begin(); bit != basis.end(); ++bit)
				{
					if (bit->second >= ubdeg)
						break;
					if (bit->first | UB)
					{
						isreducable = true;
						create_basis_multiple(bit->first, UB / bit->first, upper_bound, basisitem[bit->first], LAZY_COMPUTATIONS);
						break;
					}
				}
				if (!isreducable)
				{
					dense_invindex[upper_bound] = dense_index.size();
					dense_index.emplace_back(upper_bound);
					dense_index2.emplace(upper_bound);
				}
			}
#endif
		}

		// insert new basis poly: add new multiples for all non-reducable monomials that are now divisable by LM of this poly
		template<typename Tail>
		basisrep_t& insert_basis(const int_monomial_t& lm, Tail&& tail)
		{
			insert(lm, std::forward<Tail>(tail), lm);
			MQDEBUGPOLY(lm);

#ifdef BASISSIEVE
			const static_monomial_t LM = lm;
			const unsigned lmdeg = LM.degree();
			basis[lm] = lmdeg;
			basisrep_t& br = basisitem[lm].assign(lm);
			// find first element of dense_index with degree higher than degree(lm)
			dense_index2.erase(lm);
			int_monomial_t mul(1);
			int_monomial_t res = LM * mul;
			auto it2 = std::lower_bound(dense_index.begin(), dense_index.end(), res);
			if (it2 == dense_index.end())
				return br;

			const std::size_t dilen = dense_index.end() - it2;
			const std::size_t mucnt = int_monomial_t(upper_bound.degree() - lmdeg, maximum_of_degree_tag()).value();
			auto it = dense_index2.lower_bound(it2->intm);
			if (dilen < mucnt)
			{
				br.mul = int_monomial_t(upper_bound.degree() - lmdeg, minimum_of_degree_tag());
				br.res = LM * br.mul;
				for (; it != dense_index2.end();)
					if (LM | *it)
					{
						create_basis_multiple(lm, *it / lm, *it, br, LAZY_COMPUTATIONS);
						it = dense_index2.erase(it);
					}
					else
						++it;
			}
			else
			{
				const int_monomial_t end = *dense_index2.rbegin();
				while (res <= end)
				{
					while (*it < res)
						++it;
					if (*it == res)
					{
						create_basis_multiple(lm, mul, res, br, LAZY_COMPUTATIONS);
						it = dense_index2.erase(it);
					}
					mul = int_monomial_t(mul.value() + 1);
					res = LM * mul;
				}
				br.mul = mul;
				br.res = res;
			}
			return br;
#else
			unsigned lmdeg = lm.degree();
			basis[lm] = lmdeg;
			basisrep_t& br = basisitem[lm].assign(lm);
			// find first element of dense_index with degree higher than degree(lm)
			dense_index2.erase(lm);
			auto it = dense_index2.lower_bound(int_monomial_t(lmdeg + 1, minimum_of_degree_tag()));
			for (; it != dense_index2.end();)
				if (lm | *it)
				{
					create_basis_multiple(lm, *it / lm, *it, br, LAZY_COMPUTATIONS);
					it = dense_index2.erase(it);
				}
				else
					++it;
			return br;
#endif
		}

		void _update_matrix_entry(iterator& it, bool immediate_reduce)
		{
			// check if poly is computed and of this generation
			if (it->second.generation != generation)
			{
				if (it->second.generation == -1)
				{
					// poly was not computed yet
					it->second.tail = get_u_g(it->first / it->second.blm, it->second.blm, immediate_reduce);
					it->second.generation = generation;
#ifdef LAZY_MATRIXMULT
					auto biit = basisitem.find(it->second.blm);
					if (biit != basisitem.end())
						biit->second.matrix_multiples.insert(it->first);
#endif
				}
				else
				{
					const std::vector<int_monomial_t>& gdi = dense_index_generation[it->second.generation];
					if (!(it->second.tail.size() <= dense_index.size() && gdi[it->second.tail.size() - 1] == dense_index[it->second.tail.size() - 1].intm))
					{
						// poly was computed, but needs to be updated
						// TODO: check if recomputing is not faster
#ifdef USETHREADS
						std::vector< std::pair<coefficient_t, iterator> > todo;
#endif
						dense_poly_t newtail;
						newtail.reserve(it->second.tail.size());
						std::size_t j = 0;
						for (std::size_t i = 0; i < it->second.tail.size(); ++i)
						{
							if (it->second.tail[i] == 0)
								continue;
							while (j < dense_index.size() && dense_index[j].intm < gdi[i])
								++j;
							if (j < dense_index.size() && dense_index[j].intm == gdi[i])
							{
								if (j >= newtail.size())
									newtail.resize(j + 1);
								newtail[j] += it->second.tail[i];
							}
							else
							{
#ifdef USETHREADS
								auto rit = find(gdi[i]);
								dense_poly_t* ritptr = get_try(rit, immediate_reduce);
								if (ritptr == nullptr)
									todo.emplace_back(-it->second.tail[i], rit);
								else
									add_to(newtail, -it->second.tail[i], *ritptr);
#else
								add_to(newtail, -it->second.tail[i], get(gdi[i], immediate_reduce));
#endif
							}
						}
#ifdef USETHREADS
						for (auto& crit : todo)
							add_to(newtail, crit.first, get(crit.second, immediate_reduce));
#endif
						newtail.shrink_to_fit();
						it->second.tail = std::move(newtail);
					}
					it->second.generation = generation;
				}
			}
		}

#ifdef USETHREADS
		// get poly from polymatrix by iterator
		inline dense_poly_t* get_try(iterator& it, bool immediate_reduce = false)
		{
			// check if poly is computed and of this generation
			if (it->second.generation != generation)
			{
				// lock: let other threads wait until this work has finished
				// try_lock: if locking fails returns nullptr to allow other computation in this thread instead of waiting
				if (!it->second.mutex.try_lock())
					return nullptr;
				lock_type lock(it->second.mutex, std::adopt_lock);
				_update_matrix_entry(it, immediate_reduce);
			}
			return &it->second.tail;
		}
#endif

		// get poly from polymatrix by iterator
		inline dense_poly_t& get(iterator& it, bool immediate_reduce = false)
		{
			if (it->second.generation != generation)
			{
#ifdef USETHREADS
				// lock: let other threads wait until this work has finished
				lock_type lock(it->second.mutex);
#endif
				_update_matrix_entry(it, immediate_reduce);
			}
			return it->second.tail;
		}

		// get poly from polymatrix byLM
		inline dense_poly_t& get(const int_monomial_t& m, bool immediate_reduce = false)
		{
			auto it = find(m);
			if (it == end())
				throw std::runtime_error("matrix::get(m): polynomial not found!");
			return get(it, immediate_reduce);
		}

		// monomial 1
		const int_monomial_t _one;
		// get leading monomial of poly in dense rep
		const int_monomial_t& get_lm(const dense_poly_t& p) const
		{
			if (p.empty())
				return _one;
			return dense_index[p.size() - 1].intm;
		}
		// get leading term of poly in dense rep
		term_t get_lt(const dense_poly_t& p) const
		{
			term_t lt;
			if (!p.empty())
			{
				lt.first = p.back();
				lt.second = dense_index[p.size() - 1].intm;

				MQDEBUGCHECK(p.back() == 0)
			}
			else
			{
				lt.first = 0;
				lt.second = 0;
			}
			return lt;
		}
		// get polynomial in normal rep from polymatrix
		polynomial_t get_polynomial(const int_monomial_t& m)
		{
			const dense_poly_t& pr = get(m);

			polynomial_t tmp;
			tmp.reserve(pr.size() + 1);

			for (std::size_t i = 0; i < pr.size(); ++i)
				if (pr[i] != 0)
					tmp.add_head(pr[i], dense_index[i].intm);
			tmp.add_head(1, m);
			return tmp;
		}

		// remove any trailing zero coefficients from poly in dense rep
		void cleanup(dense_poly_t& p)
		{
			std::size_t i = p.size();
			while (i > 0 && p[i - 1] == 0)
				--i;
			p.resize(i);
		}

		// substract poly g from poly f in place
		void substract_to(dense_poly_t& f, const dense_poly_t& g)
		{
			gb::substract_to(f, g);
			cleanup(f);
		}

		// compute f += c * g
		void add_to(dense_poly_t& f, coefficient_t c, const dense_poly_t& g)
		{
			if (c == 0)
				return;
			gb::add_to(f, c, g);
			cleanup(f);
		}

		// lead reduce poly in dense rep with polymatrix
		void lead_reduce(dense_poly_t& p)
		{
			while (!p.empty())
			{
				MQDEBUGCHECK(p.size() > dense_index.size())

					std::size_t i = p.size() - 1;
				auto it = find(dense_index[i].intm);
				if (it == end())
					return;
				const coefficient_t c = -p[i];
				p[i] = 0;
				add_to(p, c, get(it));
			}
#if 0
			for (int i = (int)(p.size()) - 2; i >= 0; --i)
			{
				if (p[i] == 0)
					continue;
				auto it = find(dense_index[i].intm);
				if (it == end())
					continue;
				const coefficient_t c = -p[i];
				p[i] = 0;
				add_to(p, c, get(it));
			}
#endif
		}

		// compute fullreduce(u*g), result is poly in dense rep
		dense_poly_t get_u_g(const static_monomial_t& u, const dense_poly_t& g, bool immediate_reduce = false)
		{
			dense_poly_t ret;
			int_monomial_t maxm = u * dense_index[g.size() - 1].intm;
			auto ubit = std::lower_bound(dense_index.begin(), dense_index.end(), maxm, [](const monorep_t& l, const int_monomial_t& r) { return l.intm < r; });
			ret.reserve(ubit - dense_index.begin());

#ifdef USETHREADS
			std::vector< std::pair<coefficient_t, iterator> > todo;
#endif
			for (std::size_t i = g.size(); i > 0;)
			{
				--i;
				const coefficient_t c = g[i];
				if (c == 0)
					continue;
				const int_monomial_t m = dense_index[i].intm * u;

				MQDEBUGCHECK(m >= upper_bound);

				if (immediate_reduce)
				{
					auto rit = find(m);
					if (rit == end())
					{
						auto mit = dense_invindex.find(m);
						if (mit->second >= ret.size())
							ret.resize(mit->second + 1);
						ret[mit->second] += c;
					}
					else
					{
#ifdef USETHREADS
						dense_poly_t* ritptr = get_try(rit, true);
						if (ritptr == nullptr)
							todo.emplace_back(-c, rit);
						else
							add_to(ret, -c, *ritptr);
#else
						add_to(ret, -c, get(rit, true)); // term c*m   can be immediately reduced by  -c*rit->second
#endif
					}
				}
				else
				{
					auto mit = dense_invindex.find(m);
					if (mit != dense_invindex.end())
					{
						// store term in dense part
						if (mit->second >= ret.size())
							ret.resize(mit->second + 1);
						ret[mit->second] += c;
					}
					else
					{
						// immediately reduce
						auto rit = find(m);
						if (rit == end())
							throw std::runtime_error("polymatrix::get_u_g(): monomial not found in dense nor in matrix");
#ifdef USETHREADS
						dense_poly_t* ritptr = get_try(rit);
						if (ritptr == nullptr)
							todo.emplace_back(-c, rit);
						else
							add_to(ret, -c, *ritptr);
#else
						add_to(ret, -c, get(rit)); // term c*m   can be immediately reduced by  -c*rit->second
#endif
					}
				}
			}
#ifdef USETHREADS
			for (auto& crit : todo)
				add_to(ret, crit.first, get(crit.second, immediate_reduce));
#endif
			cleanup(ret);
			ret.shrink_to_fit();
			return ret;
		}

		// get fullreduce(u*tail(g)) with g in basis with LM(g) = glm
		inline dense_poly_t get_u_g(const static_monomial_t& u, const int_monomial_t& glm, bool immediate_reduce = false)
		{
			const dense_poly_t& g = get(glm);
			return get_u_g(u, g, immediate_reduce);
		}

		// g - LM(g)/LM(f)*f  (note LC(g)=LC(f)=1)
		dense_poly_t get_g_reduced_by_f(int_monomial_t glm, int_monomial_t flm)
		{
			// input: flm,glm in basis, flm | glm, but flm != glm
			// both monic so only have to add: g.tail - (g.lm/f.lm) * f.tail
			dense_poly_t ret = get_u_g(glm / flm, flm);
			substract_to(ret, get(glm));
			return ret;
		}

		// erase elements of v at positions indexed by pos
		template<typename Vector>
		void remove_items(Vector& v, const std::vector<std::size_t>& pos)
		{
			if (pos[0] >= v.size())
				return;
			std::size_t i = 0;
			for (; i + 1 < pos.size() && pos[i + 1] < v.size(); ++i)
			{
				for (std::size_t j = pos[i] + 1; j < pos[i + 1]; ++j)
					v[j - i - 1] = v[j];
			}
			for (; i < pos.size() && pos[i] < v.size(); ++i)
			{
				for (std::size_t j = pos[i] + 1; j < v.size(); ++j)
					v[j - i - 1] = v[j];
			}
			v.resize(v.size() - i);
			v.shrink_to_fit();
		}

		// shrink polymatrix: do full rowreduction of polymatrix, erase all reducable monomial columns
		void shrink()
		{

#ifdef LAZY_SHRINK
			// create backup copy of dense_index for previous generation
			std::vector<int_monomial_t> gdi(dense_index.size());
			for (std::size_t i = 0; i < dense_index.size(); ++i)
				gdi[i] = dense_index[i].intm;
			dense_index_generation.emplace_back(std::move(gdi));
			// increment generation
			++generation;
#endif

			std::vector<std::size_t> to_erase;
			for (std::size_t i = 0; i < dense_index.size(); ++i)
			{
				auto it = find(dense_index[i].intm);
				if (it == end())
					continue;
				to_erase.emplace_back(i);
#ifndef LAZY_SHRINK
				for (auto wit = begin(); wit != end(); ++wit)
				{
					if (wit->second.generation == generation && i < wit->second.tail.size() && wit->second.tail[i] != 0)
					{
						const coefficient_t c = -wit->second.tail[i];
						wit->second.tail[i] = 0;
						add_to(wit->second.tail, c, get(it, true));
					}
				}
#endif
			}
			if (to_erase.empty())
				return;
#ifndef LAZY_SHRINK
			for (auto wit = begin(); wit != end(); ++wit)
			{
#ifdef MQDEBUG
				for (std::size_t i = 0; i < to_erase.size() && to_erase[i] < wit->second.tail.size(); ++i)
					if (wit->second.tail[to_erase[i]] != 0)
						throw;
#endif
				if (wit->second.generation == generation)
					remove_items(wit->second.tail, to_erase);
			}
#endif
			// remove monomials in dense_index and update dense_invindex
			if ((dense_index.size() - to_erase.front()) > (dense_index.size() >> 2))
			{
				// if the fraction of positions that need to be updated is more than 25% then just do a full reindex
				remove_items(dense_index, to_erase);
				dense_invindex.clear();
				for (std::size_t i = 0; i < dense_index.size(); ++i)
					dense_invindex[dense_index[i].intm] = i;
			}
			else
			{
				// otherwise do an incremental update
				to_erase.emplace_back(std::size_t(0) - std::size_t(1)); // helper value to avoid additional range checking 
				std::size_t i = to_erase.front(), j = 0;
				for (; i < dense_index.size(); ++i)
				{
					if (i == to_erase[j])
					{
						do
						{
							dense_invindex.erase(dense_index[i].intm);
							++i; ++j;
						} while (i == to_erase[j]);
						if (i >= dense_index.size())
							break;
					}
					dense_invindex[dense_index[i].intm] -= j;
				}
				remove_items(dense_index, to_erase);
			}
		}

		// perform rowreduction of mat (no interaction with polymatrix)
		// optional postprocessing: perform polynomial reduction of current basis and toadd polys under toadd polys until no further reduction is possible
		void rowreduce(std::vector<dense_poly_t>& mat, bool nopostprocessing = false)
		{
#ifdef USETHREADS
			std::atomic_int_fast64_t index(0);
			boost::barrier barrier(threadpool.size() + 1);
			struct worker_t {
				worker_t(polymatrix& _pm, std::vector<dense_poly_t>& _mat, boost::barrier& _barrier, std::atomic_int_fast64_t& _index)
					: pm(_pm), mat(_mat), barrier(_barrier), index(_index)
				{}
				void operator()(int id)
				{
					int pivot = 0;
					while (pivot < (int)(mat.size()) && mat[pivot].empty())
						++pivot;
					int idx = index++;
					int sub = 0;
					while (pivot < (int)(mat.size()))
					{
						while (idx < (int)(mat.size()))
						{
							if (idx != pivot && mat[idx].size() >= mat[pivot].size() && mat[idx][mat[pivot].size() - 1] != 0)
								pm.add_to(mat[idx], (-mat[idx][mat[pivot].size() - 1]) / mat[pivot].back(), mat[pivot]);
							idx = index++ - sub;
						}
						sub += mat.size();
						idx -= mat.size();
						barrier.wait();
						++pivot;
						while (pivot < (int)(mat.size()) && mat[pivot].empty())
							++pivot;
					}
				}
				polymatrix& pm;
				std::vector<dense_poly_t>& mat;
				boost::barrier& barrier;
				std::atomic_int_fast64_t& index;
			};
			for (int i = 0; i < threadpool.size() + 1; ++i)
				threadpool.push(worker_t(*this, mat, barrier, index));
			threadpool.wait();
			for (int i = 0; i < (int)(mat.size());)
			{
				if (mat[i].empty())
				{
					if (i != (int)(mat.size()) - 1)
						swap(mat[i], mat.back());
					mat.pop_back();
				}
				else
					++i;
			}
#else
			std::map<std::size_t, std::vector<dense_poly_t> > len;
			for (auto& p : mat)
			{
				MQDEBUGCHECK(p.size() > dense_index.size())
					if (!p.empty())
						len[p.size()].emplace_back(std::move(p));
			}
			mat.clear();
			std::vector<dense_poly_t> tmp;
			while (!len.empty())
			{
				auto it = len.end(); --it;
				tmp = std::move(it->second);
				len.erase(it);
				for (auto it2 = tmp.begin() + 1; it2 != tmp.end(); ++it2)
				{
					MQDEBUGCHECK(it2->size() != tmp.front().size())
						add_to(*it2, (-(it2->back())) / tmp.front().back(), tmp.front());
					if (!it2->empty())
						len[it2->size()].emplace_back(std::move(*it2));
				}
				for (auto it2 = mat.begin(); it2 != mat.end(); ++it2)
					if ((*it2)[tmp.front().size() - 1] != 0)
						add_to(*it2, (-(*it2)[tmp.front().size() - 1]) / tmp.front().back(), tmp.front());
				MQDEBUGCHECK(tmp.front().size() > dense_index.size())
					mat.emplace_back(std::move(tmp.front()));
			}
#endif

			if (nopostprocessing || mat.empty())
				return;

#ifdef ROWREDUCE_POSTPROCESSING
			std::map<int_monomial_t, unsigned> oldbasis = basis;
			std::map<std::size_t, dense_poly_t> toadd_oldred;
			std::map<int_monomial_t, std::size_t> basis_divisable;
			std::map<std::size_t, dense_poly_t> divisable;
			std::map<std::size_t, dense_poly_t> toadd;
			std::unordered_map<std::size_t, std::size_t> divisor;

			for (auto& p : mat)
				toadd.emplace(p.size() - 1, std::move(p));
			mat.clear();

			dense_poly_t tmpp, tmpp2;

			auto startpostprocessing = std::chrono::system_clock::now();
			get_logger()("postproc", lg_verbose) << "cnt=" << toadd.size() << " mindeg=" << (toadd.empty() ? 0 : dense_index[toadd.begin()->first].degm) << " maxdeg=" << (toadd.empty() ? 0 : dense_index[toadd.rbegin()->first].degm) << " time=0s" << std::endl;

			// check if basis polys are lead-reducable by toadd polys
			for (auto it = oldbasis.end(); it != oldbasis.begin();)
			{
				--it;
				const unsigned blmdeg = it->second;
				for (auto it2 = toadd.begin(); it2 != toadd.end(); ++it2)
				{
					if (!(dense_index[it2->first].degm < blmdeg))
						break;
					if (!(dense_index[it2->first].intm | it->first))
						continue;
					// set as lead-reducable
					basis_divisable.emplace(it->first, it2->first);
					// remove from oldbasis
					it = oldbasis.erase(it);
					break;
				}
			}

			// check divisability of LMs between toadd polys
			for (auto it = toadd.begin(); it != toadd.end(); ++it)
			{
				const int_monomial_t m = dense_index[it->first].intm;
				const static_monomial_t M = m;
				unsigned degm = dense_index[it->first].degm;

				for (auto it2 = toadd.end(); it2 != toadd.begin();)
				{
					--it2;
					if (!(degm < dense_index[it2->first].degm))
						break;
					if (!(M | dense_index[it2->first].intm))
						continue;
					// move from toadd into divisable
					divisable.emplace(it2->first, std::move(it2->second));
					// store divisor so we don't have to find it again
					divisor[it2->first] = it->first;
					// erase from toadd
					it2 = toadd.erase(it2);
				}
			}

			std::size_t linearpolycount = 0;
			for (auto it = toadd.begin(); it != toadd.end(); ++it)
			{
				unsigned degm = dense_index[it->first].degm;
				if (degm == 1)
				{
					if (++linearpolycount == polynomial_t::max_vars)
						break;
				}
				else if (degm == 0)
					throw std::runtime_error("contradiction: 1=0");
				else
					break;
			}

			unsigned mindeg = (toadd.empty() ? 0 : dense_index[toadd.begin()->first].degm);

			while (true)
			{
				if (divisable.empty() && basis_divisable.empty())
					break;

				// reduce the smallest lead-reducable poly
				auto dvit = divisable.begin();
				auto obit = basis_divisable.begin();

				if (dvit == divisable.end() || (obit != basis_divisable.end() && obit->first < dense_index[dvit->first].intm))
				{
					/* smallest lead-reducable poly is a basis poly */
					if (linearpolycount == polynomial_t::max_vars && degree(obit->first) > 2)
						break;

					// find poly to reduce with
					std::size_t divmidx = obit->second;
					if (divmidx == 0)
						throw;
					auto rrit = toadd.find(divmidx);
					while (rrit == toadd.end())
					{
						if (divisor.count(divmidx) == 0)
							throw;
						divmidx = divisor[divmidx];
						rrit = toadd.find(divmidx);
					}

					// compute multiple of divisor poly: automatic substraction of basis poly
					tmpp = get_u_g(obit->first / dense_index[rrit->first].intm, rrit->second);

					// tmpp is further processed below

					basis_divisable.erase(obit);
				}
				else
				{
					/* smallest lead-reducable poly is a regular poly in divisable */
					if (linearpolycount == polynomial_t::max_vars && dense_index[dvit->first].degm > 2)
						break;

					// find poly to reduce with
					if (divisor.count(dvit->first) == 0)
						throw;
					std::size_t divmidx = divisor[dvit->first];
					if (divmidx == 0)
						throw;
					auto rrit = toadd.find(divmidx);
					while (rrit == toadd.end())
					{
						if (divisor.count(divmidx) == 0)
							throw;
						divmidx = divisor[divmidx];
						rrit = toadd.find(divmidx);
					}

					// compute multiple of divisor poly
					tmpp = get_u_g(dense_index[dvit->first].intm / dense_index[rrit->first].intm, rrit->second);
					// substract divisable poly
					add_to(tmpp, (-tmpp.back()) / dvit->second.back(), dvit->second);

					// tmpp is further processed below

					// move old poly into toadd_oldred to reduce other polys with in the future
					toadd_oldred.emplace(dvit->first, std::move(dvit->second));
					divisable.erase(dvit);
				}

				// need to add tmpp to either toadd or divisable, and check leadreducability of other toadd and oldbasis polys with tmpp
				if (tmpp.empty())
					continue;

				// leadreduce with toadd_oldred and divisable
				auto taorit = toadd_oldred.lower_bound(tmpp.size() - 1);
				if (taorit == toadd_oldred.end() && taorit != toadd_oldred.begin())
					--taorit;
				auto divbit = divisable.lower_bound(tmpp.size() - 1);
				if (divbit == divisable.end() && divbit != divisable.begin())
					--divbit;
				auto toit = toadd.lower_bound(tmpp.size() - 1);
				if (toit == toadd.end() && toit != toadd.begin())
					--toit;
				while (!tmpp.empty())
				{
					while (taorit != toadd_oldred.begin() && taorit->first > tmpp.size() - 1)
						--taorit;
					while (divbit != divisable.begin() && divbit->first > tmpp.size() - 1)
						--divbit;
					while (toit != toadd.begin() && toit->first > tmpp.size() - 1)
						--toit;
					if (toit->first == tmpp.size() - 1)
						add_to(tmpp, (-tmpp.back()) / toit->second.back(), toit->second);
					else if (taorit->first == tmpp.size() - 1)
						add_to(tmpp, (-tmpp.back()) / taorit->second.back(), taorit->second);
					else if (divbit->first == tmpp.size() - 1)
						add_to(tmpp, (-tmpp.back()) / divbit->second.back(), divbit->second);
					else
						break;
				}

				// rowreduce with toadd
#ifndef POSTPROCESSING_ONLYLEADREDUCE
				for (auto rit = toadd.begin(); rit != toadd.end() && rit->first < tmpp.size(); ++rit)
					if (tmpp[rit->first] != 0)
						add_to(tmpp, (-tmpp[rit->first]) / rit->second.back(), rit->second);
#endif
				if (tmpp.empty())
					continue;

				const unsigned degm = dense_index[tmpp.size() - 1].degm;
				const static_monomial_t M = dense_index[tmpp.size() - 1].intm;

				if (degm < mindeg)
				{
					get_logger()("postproc", lg_verbose) << "cnt=" << toadd.size() << " mindeg=" << (toadd.empty() ? 0 : dense_index[toadd.begin()->first].degm) << " maxdeg=" << (toadd.empty() ? 0 : dense_index[toadd.rbegin()->first].degm) << " time=" << std::chrono::duration<double>(std::chrono::system_clock::now() - startpostprocessing).count() << "s" << std::endl;
					mindeg = degm;
				}
				if (degm == 1)
					++linearpolycount;
				else if (degm == 0)
					throw std::runtime_error("contradiction: 1=0");

				// rowreduce toadd with tmpp
				bool leadreducable = false;
				for (auto rit = toadd.begin(); rit != toadd.end(); ++rit)
				{
#ifndef POSTPROCESSING_ONLYLEADREDUCE
					if (rit->first >= tmpp.size() && rit->second[tmpp.size() - 1] != 0)
						add_to(rit->second, (-rit->second[tmpp.size() - 1]) / tmpp.back(), tmpp);
					if (dense_index[rit->first].degm < degm && !leadreducable && (dense_index[rit->first].intm | M))
					{
						leadreducable = true;
						divisor[tmpp.size() - 1] = rit->first;
						divisable.emplace(tmpp.size() - 1, std::move(tmpp));
					}
#else
					if (dense_index[rit->first].degm >= degm)
						break;
					if (!leadreducable && (dense_index[rit->first].intm | M))
					{
						leadreducable = true;
						divisor[tmpp.size() - 1] = rit->first;
						divisable.emplace(tmpp.size() - 1, std::move(tmpp));
						break;
					}
#endif
				}

				if (leadreducable)
					continue;


				// check if remaining basis polys are lead-reducable by this new addition
				for (auto bit = oldbasis.end(); bit != oldbasis.begin();)
				{
					--bit;
					if (!(degm < bit->second))
						break;
					if (!(M | bit->first))
						continue;
					basis_divisable.emplace(bit->first, tmpp.size() - 1);
					bit = oldbasis.erase(bit);
				}
				// check if other toadd are lead-reducable by this new addition
				for (auto rit = toadd.end(); rit != toadd.begin();)
				{
					--rit;
					if (!(degm < dense_index[rit->first].degm))
						break;
					if (!(M | dense_index[rit->first].intm))
						continue;

					divisable.emplace(rit->first, std::move(rit->second));
					divisor[rit->first] = tmpp.size() - 1;
					rit = toadd.erase(rit);
				}
				toadd.emplace(tmpp.size() - 1, std::move(tmpp));
			}
			get_logger()("postproc", lg_verbose) << "cnt=" << toadd.size() << " mindeg=" << (toadd.empty() ? 0 : dense_index[toadd.begin()->first].degm) << " maxdeg=" << (toadd.empty() ? 0 : dense_index[toadd.rbegin()->first].degm) << " time=" << std::chrono::duration<double>(std::chrono::system_clock::now() - startpostprocessing).count() << "s" << std::endl;
			for (auto& p : toadd)
				mat.emplace_back(std::move(p.second));

#endif // ROWREDUCE_POSTPROCESSING
		}



		void get_divisors(std::vector<int_monomial_t>& divisors, const static_monomial_t& M, bool strictdivisors = false)
		{
			divisors.clear();
			if (!strictdivisors)
				divisors.emplace_back(M);
			if (M.count() == 0)
				return;
			if (M.count() == 1)
			{
				static_monomial_t div = M;
				for (--div._data[0].second; div._data[0].second != 0; --div._data[0].second)
					divisors.emplace_back(div);
				divisors.emplace_back(0);
				return;
			}

			const int degM = (int)(M.degree());
			std::vector< std::vector<int_monomial_t> > deg_M(degM + 1);
			deg_M[degM].emplace_back(M);
			static_monomial_t tmp, tmp2;
			for (int deg = degM; deg > 1; --deg)
			{
				for (auto& m : deg_M[deg])
				{
					tmp = m;
					for (int i = 0; i < tmp.count(); ++i)
					{
						if ((--tmp._data[i].second) == 0)
						{
							tmp2._size = tmp._size - 1;
							for (int j = 0; j < i; ++j)
								tmp2._data[j] = tmp._data[j];
							for (int j = i + 1; j < tmp.count(); ++j)
								tmp2._data[j - 1] = tmp._data[j];
							deg_M[deg - 1].emplace_back(tmp2);
						}
						else
							deg_M[deg - 1].emplace_back(tmp);
						++tmp._data[i].second;
					}
				}
				std::sort(deg_M[deg - 1].begin(), deg_M[deg - 1].end(), std::greater<int_monomial_t>());
				deg_M[deg - 1].erase(std::unique(deg_M[deg - 1].begin(), deg_M[deg - 1].end()), deg_M[deg - 1].end());
				for (auto& m : deg_M[deg - 1])
					divisors.emplace_back(m);
			}
			divisors.emplace_back(0);
		}

	};





	template<typename poly_t>
	class m4gb
		: public solver_base_t<poly_t>
	{
	public:
		typedef typename solver_base_t<poly_t>::polynomial_t polynomial_t;
		typedef typename polynomial_t::term_t                term_t;
		typedef typename polynomial_t::coefficient_t         coefficient_t;
		typedef typename polynomial_t::int_monomial_t        int_monomial_t;
		typedef typename polynomial_t::static_monomial_t     static_monomial_t;
		typedef typename polynomial_t::dynamic_monomial_t    dynamic_monomial_t;

		typedef typename polynomial_t::traits_t::less_t monomial_less_t;

		typedef typename polynomial_t::const_iterator::value_type::second_type ref_monomial_t;

		typedef std::vector<coefficient_t> nonred_poly_t;
		typedef std::unordered_map<int_monomial_t, nonred_poly_t> matrix_type;

		typedef polymatrix<polynomial_t> polymatrix_t;
		typedef typename polymatrix_t::dense_poly_t dense_poly_t;
		typedef typename polymatrix_t::polyrep_less_t polyrep_less_t;
		typedef typename polymatrix_t::basisrep_t basisrep_t;

		struct crit_pair_t
		{
			int_monomial_t p1;
			int_monomial_t p2;
			int_monomial_t intlcm;
			unsigned lcmdeg;

			crit_pair_t(const int_monomial_t& _p1, const int_monomial_t& _p2)
				: p1(_p1), p2(_p2)
			{
				if (p2 < p1)
					std::swap(p1, p2);
				static_monomial_t M = gb::lcm(p1, p2);
#ifdef MAXDEGREE
				intlcm = M.degree() > MAXDEGREE ? int_monomial_t() : int_monomial_t(M);
#else
				intlcm = M;
#endif
				lcmdeg = intlcm.degree();
			}
			bool operator<(const crit_pair_t& r) const
			{
				if (intlcm != r.intlcm)
					return intlcm < r.intlcm;
				return p2 < r.p2;
			}
		};

		polymatrix_t matrix;
		std::multiset<crit_pair_t> CP;
		std::multiset<dense_poly_t, polyrep_less_t> to_add;
		polyrep_less_t _polyrepless;
		std::unordered_map<int_monomial_t, std::set<int_monomial_t> > sel_lcm_lmpoly;
		std::vector<dense_poly_t> submatrix;

#ifdef MAXDEGREE
		unsigned nremoved_cp;
#endif

#ifdef USE_CP_SIMPLIFY
		// f.lm => ( u => (v, g.lm)   whenever u*f.lm = LCM(f.lm,g.lm) = v*g.lm and Red(Spoly(f,g))==0
		std::unordered_map<int_monomial_t, boost::container::flat_set<int_monomial_t> > CP_simplify;

		void cp_simplify_add(int_monomial_t flm, int_monomial_t glm, int_monomial_t fglcm)
		{
//			if (! (lcm(flm, glm) | fglcm ))
//			{
//				std::cout << flm << " * " << glm << " != " << fglcm << std::endl;
//				throw;
//			}
			CP_simplify[flm].insert(glm);
			CP_simplify[glm].insert(flm);
		}

		void cp_simplify_add(const crit_pair_t& cp)
		{
			cp_simplify_add(cp.p1, cp.p2, cp.intlcm);
		}

		std::set<int_monomial_t> cp_simplify(int_monomial_t cplcm, int_monomial_t flm)
		{
			std::set<int_monomial_t> ret;
			std::vector<int_monomial_t> retnew;
			ret.insert(flm);
			retnew.emplace_back(flm);
			while (!retnew.empty())
			{
				int_monomial_t glm = retnew.back();
				retnew.pop_back();

				auto git = CP_simplify.find(glm);
				if (git == CP_simplify.end())
					continue;
				for (auto hlm : git->second)
					if ((hlm | cplcm) && ret.insert(hlm).second)
						retnew.emplace_back(hlm);
			}
			return ret;
		}

		bool cp_simplify_check_and_add(int_monomial_t flm, int_monomial_t glm, int_monomial_t fglcm)
		{
			bool todo = true;
			auto fset = cp_simplify(fglcm, flm);
			auto gset = cp_simplify(fglcm, glm);

			// if intersection is nonempty then Red(Spoly(f,g)) = 0
			for (auto& i : fset)
				if (gset.count(i))
				{
					todo = false;
					break;
				}

			if (todo)
			{
				unsigned lcmdeg = degree(fglcm);
				for (auto& i : fset)
				{
					unsigned ideg = degree(i);
					for (auto& j : gset)
						if (degree(j) + ideg == lcmdeg || lcm(i, j) != fglcm)
						{
							todo = false;
							break;
						}
					if (!todo)
						break;
				}
			}
			cp_simplify_add(flm, glm, fglcm);
			return todo;
		}

#endif

		double totalupdatetime, totalmatrixtime, totalselectiontime;

		// temporaries
		std::set<int_monomial_t> _oldbasis;

		m4gb()
			: matrix(), _polyrepless()
		{
			this->_solvername = "m4gb";
		}

		virtual void solve()
		{
#ifdef USETHREADS
			matrix.threadpool.resize(this->nrthreads - 1);
#endif

			totalupdatetime = totalmatrixtime = totalselectiontime = 0;

			initialize();
			this->msg() << " I " << matrix.basis.size() << " " << CP.size() << " " << to_add.size() << " " << matrix.matrix.size() << std::endl;
			while (CP.size() + to_add.size() > 0)
			{
				auto startupdate = std::chrono::system_clock::now();

				update();
				auto endupdate = std::chrono::system_clock::now();
				double updatetime = std::chrono::duration<double>(endupdate - startupdate).count();
				totalupdatetime += updatetime;
				this->msg() << " U #b:" << std::setw(4) << matrix.basis.size() << " #cp:" << std::setw(6) << CP.size() << " #mr:" << std::setw(5) << matrix.matrix.size() << " #mc:" << std::setw(5) << matrix.dense_index.size() << " s=" << updatetime << std::endl;

				matrixprepare();
				auto endmatrix = std::chrono::system_clock::now();
				double matrixtime = std::chrono::duration<double>(endmatrix - endupdate).count();
				totalmatrixtime += matrixtime;
				this->msg() << " M #b:" << std::setw(4) << matrix.basis.size() << " #cp:" << std::setw(6) << CP.size() << " #mr:" << std::setw(5) << matrix.matrix.size() << " #mc:" << std::setw(5) << matrix.dense_index.size() << " s=" << matrixtime << std::endl;

				if (CP.size() + to_add.size() == 0)
					break;

				selection();
				auto endselection = std::chrono::system_clock::now();
				double selectiontime = std::chrono::duration<double>(endselection - endmatrix).count();
				totalselectiontime += selectiontime;
				this->msg() << " P #b:" << std::setw(4) << matrix.basis.size() << " #cp:" << std::setw(6) << CP.size() << " #mr:" << std::setw(5) << matrix.matrix.size() << " #mc:" << std::setw(5) << matrix.dense_index.size() << " s=" << selectiontime << std::endl;
				this->msg() << " P #n:" << to_add.size() << std::endl;
			}
			this->msg() << " upper_bound = " << matrix.upper_bound.value() << " = " << matrix.upper_bound << std::endl;
			this->msg() << " total update time   : " << totalupdatetime << "s" << std::endl;
			this->msg() << " total matrix time   : " << totalmatrixtime << "s" << std::endl;
			this->msg() << " total selection time: " << totalselectiontime << "s" << std::endl;
			this->msg() << " total time          : " << totalupdatetime + totalmatrixtime + totalselectiontime << "s" << std::endl;
#ifdef MAXDEGREE
			this->msg() << " total CP removed    : " << nremoved_cp << std::endl;
#endif
			this->solution.clear();
			for (auto& glm : matrix.basis)
				this->solution.emplace_back(matrix.get_polynomial(glm.first));
		}

		void initialize()
		{
			// clear data structures
			matrix.clear();
			CP.clear();
			to_add.clear();
			sel_lcm_lmpoly.clear();
			submatrix.clear();

#ifdef MAXDEGREE
			nremoved_cp = 0;
#endif

#ifdef GB_ADD_FIELDEQUATIONS
			for (unsigned i = 0; i < polynomial_t::max_vars; ++i)
			{
				typename polynomial_t::term_t terms[2];
				terms[0] = typename polynomial_t::term_t(1, typename polynomial_t::monomial_t::pair_t(i, 1));
				terms[1] = typename polynomial_t::term_t(1, typename polynomial_t::monomial_t::pair_t(i, this->fieldsize));
				polynomial_t p(terms + 0, terms + 2);
				this->input.emplace_back(std::move(p));
			}
#endif

			std::vector<dense_poly_t> mat;
			for (auto& p : this->input)
			{
				matrix.increase_upper_bound(p.leading_monomial());
				dense_poly_t tmp;
				tmp.resize(matrix.dense_invindex[p.leading_monomial()] + 1);
				for (auto& t : p)
					tmp[matrix.dense_invindex[t.second]] = t.first;
				matrix.cleanup(tmp);
				mat.emplace_back(std::move(tmp));
			}
#ifdef PREROWREDUCE
			matrix.rowreduce(mat);
#endif
			for (auto& p : mat)
				to_add.emplace(std::move(p));
		}

		void update()
		{
			// shortcut: check for 1 != 0
			if (!to_add.empty() && matrix.dense_index[to_add.begin()->size() - 1].degm == 0)
				throw std::runtime_error("contradiction: I=(1)");

			// shortcut: check for completely linear solution for variables
			if (to_add.size() == polynomial_t::max_vars && matrix.dense_index[to_add.rbegin()->size() - 1].degm <= 1)
			{
				matrix.matrix.clear();
				matrix.basis.clear();
				matrix.basisitem.clear();
				CP.clear();
				matrix.decrease_upper_bound(matrix.dense_index[to_add.rbegin()->size() - 1].intm);
			}

			while (!to_add.empty())
			{
				// grab largest p in to_add_reduced
				auto it = to_add.end(); --it;
				dense_poly_t p;
				container_extract(to_add, it, p);
				update(p);
			}

			if (!CP.empty())
			{
				const int_monomial_t max1 = CP.rbegin()->intlcm;
				if (max1 < matrix.upper_bound)
				{
					const int_monomial_t max2 = matrix.basis.rbegin()->first;
					matrix.decrease_upper_bound(max1 > max2 ? max1 : max2);
				}
			}
			else
			{
				matrix.decrease_upper_bound(matrix.basis.rbegin()->first);
			}
		}

		void matrixprepare()
		{
			matrix.shrink();
		}


		// update basis, CP, matrix with poly p
		void update(dense_poly_t& p)
		{
			// check leading term
			term_t plt = matrix.get_lt(p);
			if (plt.first == 0)
				throw std::runtime_error("update(p): p = 0");
			if (plt.second.size() == 0)
				throw std::runtime_error("contradiction: I=(1)");

			// make monic
			if (plt.first == 1)
			{
			}
			else if (-plt.first == 1)
			{
				for (std::size_t i = 0; i < p.size(); ++i)
					p[i] = -p[i];
			}
			else
			{
				coefficient_t lcinv = coefficient_t(1) / plt.first;
				mul_to(p, lcinv);
			}
			MQDEBUGCHECK(p.back() != 1)

			// split into lm and tail
			int_monomial_t plm = plt.second;
			static_monomial_t pLM(plm);
			unsigned plmdeg = pLM.degree();
			p.pop_back();
			matrix.cleanup(p);
			p.shrink_to_fit();

			// insert into basis, matrix
			basisrep_t& basisitem = matrix.insert_basis(plm, std::move(p));


			// reduce to_add
			std::vector<dense_poly_t> newtoadd;
			for (auto it = to_add.begin(); it != to_add.end();)
			{
				MQDEBUGCHECK(it->size() > matrix.dense_index.size())
					if (matrix.find(matrix.get_lm(*it)) != matrix.end())
					{
						newtoadd.emplace_back(std::move(*it));
						it = to_add.erase(it);
					}
					else
						++it;
			}
			for (auto& g : newtoadd)
			{
				matrix.lead_reduce(g);
				if (g.empty())
					continue;
				MQDEBUGCHECK(g.size() > matrix.dense_index.size())
					to_add.emplace(std::move(g));
			}
			newtoadd.clear();

			for (auto it = CP.begin(); it != CP.end();)
			{
				if (plmdeg < it->lcmdeg && pLM | it->intlcm
					&& lcm(pLM, it->p2) != it->intlcm
					&& lcm(pLM, it->p1) != it->intlcm
					)
				{
#ifdef USE_CP_SIMPLIFY
					cp_simplify_add(*it);
					it = CP.erase(it);
#endif
				}
				else
					++it;
			}

			std::map<crit_pair_t, bool> newCP_good;
			for (auto it = matrix.basis.begin(); it != matrix.basis.end(); ++it)
			{
				if (it->first == plm)
					continue;
				crit_pair_t newCP(plm, it->first);
#ifdef MAXDEGREE
				if (newCP.intlcm.empty())
				{
					++nremoved_cp;
					continue;
				}
#endif
				if (plmdeg + it->second == newCP.lcmdeg)
				{
#ifdef USE_CP_SIMPLIFY
					cp_simplify_add(newCP);
#endif
					newCP_good.emplace(std::move(newCP), false); // bad
				}
				else
					newCP_good.emplace(std::move(newCP), true); // good
			}
			for (auto it = newCP_good.begin(); it != newCP_good.end(); ++it)
			{
				if (!it->second)
					continue;
				for (auto it2 = newCP_good.begin(); it2 != newCP_good.end() && it2->first.lcmdeg <= it->first.lcmdeg; ++it2)
					if (it != it2 && it2->first.intlcm | it->first.intlcm)
					{
#ifdef USE_CP_SIMPLIFY
//						if (it2->first.intlcm != it->first.intlcm)
//							cp_simplify_add(it->first);
#endif
						it->second = false;
						break;
					}
			}
			for (auto cp : newCP_good)
				if (cp.second)
					CP.emplace(std::move(cp.first));

			for (auto it = matrix.basis.begin(); it != matrix.basis.end();)
			{
				if (it->first == plm)
				{
					++it;
					continue;
				}
				if (plmdeg < it->second && pLM.divides(it->first))
				{
					const int_monomial_t glm = it->first;
					auto git = matrix.basisitem.find(glm);

					// 'steal' all multiples of g in M
					for (auto i : git->second.matrix_multiples)
					{
#ifndef IMMEDIATE_BASIS_REDUCE
						if (i != glm)
#endif
							basisitem.matrix_multiples.insert(i);
					}

#ifdef IMMEDIATE_BASIS_REDUCE
#ifdef USE_CP_SIMPLIFY
					cp_simplify_add(it->first, plm, it->first);
#endif
					CP.erase(crit_pair_t(plm, it->first));
					dense_poly_t tmp = matrix.get_g_reduced_by_f(glm, plm);
					matrix.lead_reduce(tmp);
					if (!tmp.empty())
					{
						MQDEBUGCHECK(tmp.size() > matrix.dense_index.size())
							to_add.emplace(std::move(tmp));
					}
#endif

					matrix.basisitem.erase(git);
					it = matrix.basis.erase(it);
				}
				else
					++it;
			}
		}


		void selection()
		{
			auto starttime = std::chrono::system_clock::now();
			get_logger()("selection", lg_verbose) << "#CP=" << CP.size() << " s=" << std::chrono::duration<double>(std::chrono::system_clock::now() - starttime).count() << std::endl;

			if (CP.empty())
				return;

			// do selection
			auto CPit = CP.begin();
			const unsigned lcmdegree = CPit->lcmdeg;
			this->msg() << " P lcmdegree=" << lcmdegree << std::endl;

			int_monomial_t lastoflcmdeg(lcmdegree, maximum_of_degree_tag());
			matrix.increase_upper_bound( lastoflcmdeg );

			get_logger()("selection", lg_verbose) << "increased upper_bound to " << lastoflcmdeg << " s=" << std::chrono::duration<double>(std::chrono::system_clock::now() - starttime).count() << std::endl;

			int_monomial_t lastlcm;
			std::size_t cnt = 0;
			while (true)
			{
#ifdef USE_CP_SIMPLIFY
				const int_monomial_t intlcm = CPit->intlcm;

#ifndef MATRIXSWAP
				matrix.increase_upper_bound(intlcm);
				auto mit = matrix.find(intlcm);
				int_monomial_t blm = mit->second.blm;
				if (cp_simplify_check_and_add(blm, CPit->p1, intlcm))
					sel_lcm_lmpoly[intlcm].insert(CPit->p1);
				if (cp_simplify_check_and_add(blm, CPit->p2, intlcm))
					sel_lcm_lmpoly[intlcm].insert(CPit->p2);
#else
				if (cp_simplify_check_and_add(CPit->p1, CPit->p2, intlcm))
				{
					sel_lcm_lmpoly[intlcm].insert(CPit->p1);
					sel_lcm_lmpoly[intlcm].insert(CPit->p2);
				}
#endif

#else
				auto& v = sel_lcm_lmpoly[CPit->intlcm];
				v.insert(CPit->p1);
				v.insert(CPit->p2);
#endif
				if (CPit->intlcm > lastlcm)
					lastlcm = CPit->intlcm;
				if (++CPit == CP.end() || lcmdegree != CPit->lcmdeg)
					break;

#ifdef MAXSELECTION
				if (++cnt == MAXSELECTION)
					break;
#endif
			}
			CP.erase(CP.begin(), CPit);

			get_logger()("selection", lg_verbose) << "cnt=" << cnt << " lastlcm=" << lastlcm << " (#=" << lastlcm.value() << ") s=" << std::chrono::duration<double>(std::chrono::system_clock::now() - starttime).count() << std::endl;

			matrix.increase_upper_bound(lastlcm);

			get_logger()("selection", lg_verbose) << "increased upper_bound to lastlcm s=" << std::chrono::duration<double>(std::chrono::system_clock::now() - starttime).count() << std::endl;

			// compute polynomials for selection
			submatrix.reserve(cnt * 2);
			std::vector<int_monomial_t> divisors;
			for (auto& lcm_lm : sel_lcm_lmpoly)
			{
				static_monomial_t LCM(lcm_lm.first);
#ifndef MATRIXSWAP
				auto mit = matrix.find(lcm_lm.first);
#ifndef USETHREADS
				const dense_poly_t& matrixrow = matrix.get(mit);
#endif
				for (auto& lm : lcm_lm.second)
				{
					int_monomial_t u, g = lm;
#if 1
					if (lm == mit->second.blm)
						continue;
#else
					auto mmit = matrix.basisitem.find(lm);
					if (mmit != matrix.basisitem.end())
					{
						if (!mmit->second.matrix_multiples.insert(lcm_lm.first).second)
							continue;
//						int_monomial_t mr_lcm = lcm(mit->second.blm, lm);
//						if (mr_lcm != lcm_lm.first && !mmit->second.matrix_multiples.insert(mr_lcm).second)
//							continue;
#ifdef SELECTION_USE_LARGEST_AVAILABLE_BASIS_MULTIPLE
						matrix.get_divisors(divisors, LCM / lm, true);
						for (auto& d : divisors)
						{
							int_monomial_t g2 = d * lm;
							if (mmit->second.matrix_multiples.count(g2) != 0)
							{
								g = g2;
								break;
							}
						}
#endif
					}
#endif
					u = LCM / g;

#ifdef USETHREADS
					submatrix.emplace_back();
					dense_poly_t& dst = submatrix.back();
					auto& matrix2 = matrix;
					const int_monomial_t lcm = lcm_lm.first;
					auto f = [&dst, lcm, u, g, &matrix2](int id)
					{
						dst = matrix2.get_u_g(u, g);
						matrix2.substract_to(dst, matrix2.get(lcm));
					};
					matrix.threadpool.push(f);
#else
					// since we process the difference, we now 'own' the matrix value also
					submatrix.emplace_back(matrix.get_u_g(u, g));
					matrix.substract_to(submatrix.back(), matrixrow);

					MQDEBUGCHECK(submatrix.back().size() > matrix.dense_index.size())
#endif
				}
#else
				bool havematrixrow = false;
				std::size_t off = submatrix.size();
				for (auto& lm : lcm_lm.second)
				{
					auto mmit = matrix.basisitem.find(lm);
					if (mmit != matrix.basisitem.end())
					{
						if (!mmit->second.matrix_multiples.insert(lcm_lm.first).second)
						{
							havematrixrow = true;
							continue;
						}
					}
					submatrix.emplace_back(matrix.get_u_g(LCM / lm, lm));
					MQDEBUGCHECK(submatrix.back().size() > matrix.dense_index.size())
				}
				auto mit = matrix.find(lcm_lm.first);
				if (havematrixrow || mit->second.computed_tail)
				{
					const dense_poly_t& matrixrow = matrix.get(mit);
					for (; off < submatrix.size(); ++off)
						matrix.substract_to(submatrix[off], matrixrow);
				}
				else
				{
					mit->second.computed_tail = true;
					mit->second.tail = std::move(submatrix.back());
					submatrix.pop_back();
#ifndef LAZY_MATRIXMULT
					matrix.basisitem[mit->second.blm].matrix_multiples.erase(lcm_lm.first);
#endif
					for (; off < submatrix.size(); ++off)
						matrix.substract_to(submatrix[off], mit->second.tail);
				}
#endif
			}

#ifdef USETHREADS
			matrix.threadpool.wait();
#endif

			get_logger()("selection", lg_verbose) << "computed matrix rows cnt=" << submatrix.size() << " s=" << std::chrono::duration<double>(std::chrono::system_clock::now() - starttime).count() << std::endl;

			matrix.rowreduce(submatrix);

			get_logger()("selection", lg_verbose) << "rowreduced matrix cnt=" << submatrix.size() << " s=" << std::chrono::duration<double>(std::chrono::system_clock::now() - starttime).count() << std::endl;

#ifdef TWEAK_SKIP_SAMEDEGREELCM_AFTER_ZEROTOADD
			if (submatrix.size() < cnt)
			{
				auto it = CP.begin();
				while (it != CP.end() && it->lcmdeg == lcmdegree)
					it = CP.erase(it);
			}
#endif

			for (auto& p : submatrix)
			{
				MQDEBUGCHECK(p.size() > matrix.dense_index.size())
					if (!p.empty())
						to_add.emplace(std::move(p));
			}

			submatrix.clear();
			sel_lcm_lmpoly.clear();
		}

	};


	struct mysolver_t
		: public m4gb<myintpolynomial_t>
	{
	};

}
#endif
