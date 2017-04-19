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

#ifndef M4GB_SOLVER_BASE_HPP
#define M4GB_SOLVER_BASE_HPP


#include "parser.hpp"
#include "logger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <map>

namespace gb {

	/* generic solver interface that is polynomial_t independent */
	class solver_t
	{
	protected:
		logger_t& _logger;
		std::string _solvername;
	public:
		solver_t(const std::string& solvername = "solver")
			: _logger(get_logger()), _solvername(solvername)
		{}

		virtual ~solver_t() {}

		virtual void clear() = 0;

		virtual void read_file(const std::string& path, poly_format pf = pf_default) = 0;

		virtual void addpoly_string(const std::string& polystr) = 0;

		virtual void addpoly_coefficients_degrevlex(const std::vector<std::size_t>& coefficients, unsigned nrvars) = 0;

		virtual void solve() = 0;

		virtual void save_solution(const std::string& path, poly_format pf = pf_default) = 0;

		virtual void print_solution(poly_format pf = pf_default) = 0;

		const std::string& name() const { return _solvername; }

		logger_t::logger_stream_t msg(log_level ll = lg_info)
		{
			return _logger(_solvername, ll);
		}

		void set_log_out(std::ostream& o)
		{
			_logger.set_out(o);
		}

		void set_log_level(log_level ll)
		{
			_logger.set_log_level(ll);
		}

		void set_log_level_out(log_level ll, std::ostream& o)
		{
			_logger.set_log_level_out(ll, o);
		}

		void clear_log_level_out(log_level ll)
		{
			_logger.clear_log_level_out(ll);
		}
	};

	/* common solver implementation for given polynomial_t */
	template<typename Poly>
	class solver_base_t
		: public solver_t
	{
	public:
		typedef Poly polynomial_t;
		typedef typename polynomial_t::field_t field_t;
		typedef typename polynomial_t::monomial_t monomial_t;
		typedef typename polynomial_t::coefficient_t coefficient_t;
		typedef typename polynomial_t::term_t  term_t;

		static const std::size_t max_vars = polynomial_t::max_vars;
		static const std::size_t fieldsize = field_t::fieldsize;
		static const std::size_t fieldchar = field_t::fieldchar;

		parser_t<polynomial_t> parser;
		std::vector<polynomial_t> input, solution;
		unsigned nrthreads;

		solver_base_t(const std::string& solvername = "solver")
			: solver_t(solvername), nrthreads(1)
		{}

		virtual ~solver_base_t()
		{}

		virtual void clear()
		{
			parser.clear();
			input.clear();
			solution.clear();
		}

		virtual void read_file(const std::string& path, poly_format pf = pf_default)
		{
			parser.read_file(path, pf);
			input = std::move(parser.polynomials);
		}

		virtual void addpoly_string(const std::string& polystr)
		{
			input.emplace_back(parser.parse_string(polystr));
		}

		void addpoly_coefficients_degrevlex(const std::vector<coefficient_t>& coefficients, unsigned nrvars = max_vars)
		{
			input.emplace_back(parser.parse_coefficients_degrevlex(coefficients, nrvars));
		}

		virtual void addpoly_coefficients_degrevlex(const std::vector<std::size_t>& coefficients, unsigned nrvars)
		{
			std::vector<coefficient_t> tmp;
			tmp.reserve(coefficients.size());
			for (auto c : coefficients)
				tmp.emplace_back((coefficient_t)(c));
			addpoly_coefficients_degrevlex(tmp, nrvars);
		}

		virtual void save_solution(const std::string& path, poly_format pf = pf_default)
		{
			std::ofstream ofs(path);
			switch (pf)
			{
			case pf_default:
				write_solution_default(ofs);
				break;

			default:
				throw std::runtime_error("solver::save_solution(): output format not supported!");
			}
		}

		virtual void print_solution(poly_format pf = pf_default)
		{
			switch (pf)
			{
			case pf_default:
				write_solution_default(std::cout);
				break;

			default:
				throw std::runtime_error("solver::print_solution(): output format not supported!");
			}
		}

		void write_solution_default(std::ostream& o)
		{
			struct {
				bool operator()(const polynomial_t& l, const polynomial_t& r) const
				{
					if (l.empty())
						return !r.empty();
					if (r.empty())
						return false;
					return l.leading_monomial() < r.leading_monomial();
				}
			} polyless;
			std::sort(solution.begin(), solution.end(), polyless);
			for (auto p : solution)
			{
				if (!p.empty() && p.leading_coefficient() != 0)
				{
					polynomial_t tmp(p);
					tmp *= coefficient_t(1) / tmp.leading_coefficient();
					o << parser.polynomial_to_string(tmp) << std::endl;
				}
				else
					o << parser.polynomial_to_string(p) << std::endl;
			}
		}

	};

	template<typename Poly>
	class dummy_solver_t
		: public solver_base_t<Poly>
	{
	public:
		dummy_solver_t()
			: solver_base_t<Poly>("dummysolver")
		{}

		virtual ~dummy_solver_t()
		{}

		virtual void solve()
		{
			this->msg(lg_abort) << "solve() called on dummy_solver_t!" << std::endl;
		}
	};

} // namespace gb

#endif // M4GB_SOLVER_BASE_HPP
