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

#ifndef M4GB_SOLVER_OPENF4_HPP
#define M4GB_SOLVER_OPENF4_HPP

#include "config.hpp"
#include "../lib/gf_elem_simple.hpp"
#include "../lib/monomial_degrevlex.hpp"
#include "../lib/polynomial_simple.hpp"
#include "../lib/solver_base.hpp"
#include "../contrib/string_algo.hpp"

#include <libopenf4.h>

#define VERBOSE 3

#if (FIELDSIZE > 2) && ((FIELDSIZE % 2) == 0)
#define GF2EXTENSION
#endif

namespace gb
{
	namespace sa = ::string_algo;

	template<typename polynomial_t>
	class openf4solver : public solver_base_t<polynomial_t>
	{
		public:
			typedef typename polynomial_t::coefficient_t gfelem_t;
			typedef typename polynomial_t::monomial_t monomial_t;

		private:
			std::vector<std::string> var_names;
			std::vector<std::string> polynomial_array;
#ifdef GF2EXTENSION
			const std::string gen_name = "t";
			std::string modulus_str;

			std::string _int_to_poly_repr(unsigned n)
			{
				std::stringstream ss;
				unsigned msb_index = detail::bitscanreverse(n);

				bool first_found = false;
				for(unsigned i=0; i <= msb_index; ++i)
				{
					if (n & (1 << i))
					{
						if (first_found) ss << "+";
						first_found = true;

						if (i == 0) { ss << 1; continue;}
						
						ss << gen_name;
						if (i > 1) ss << "^" << std::to_string(i);
					}
				}

				return ss.str();
			}

			std::string _poly_to_int_repr(const std::string & c)
			{
				std::string temp = c.substr(1, c.length() - 2); //remove "(" and ")"
				unsigned _int_repr = 0;

				if (temp[0] == '+')
					temp.erase(0, 1);

				std::vector<std::string> terms;

				sa::split(terms, temp, '+');
				for(auto term : terms)
				{
					if (term == "1")
						_int_repr += 1;
					else if (term == gen_name)
						_int_repr += 2;
					else
					{
						char t;
						unsigned e;

						sscanf(term.c_str(), "%c^%u", &t, &e);

						_int_repr += (1 << e);
					}
				}

				return std::to_string(_int_repr);
			}
#endif

			bool _is_var_name(const std::string & s) const
			{
				std::vector<std::string> var_exp_str;
				sa::split(var_exp_str, s, '^');

				return (std::find(this->var_names.begin(), this->var_names.end(), var_exp_str[0]) \
						!= this->var_names.end()) ? true : false;
			}

		public:
			openf4solver()
			{
#ifdef GF2EXTENSION
				modulus_str = _int_to_poly_repr(myfield_t::gfpoly);
#endif
			}

			void solve()
			{
				for(unsigned i=0; i < MAXVARS; ++i)
					var_names.emplace_back(this->parser.var_name(i));

				for(auto f : this->input)
				{
					std::string s, temp;
					std::stringstream ss(this->parser.polynomial_to_string(f));

					while(ss >> temp)
					{
						if (temp != "+")
						{
							std::vector<std::string> term_str;
							sa::split(term_str, temp, '*');

							if (term_str.size() > 1)
							{
								if (_is_var_name(term_str[0]))
									std::reverse(term_str.begin(), term_str.end());
								else
									std::reverse(term_str.begin()+1, term_str.end());
							}

#ifdef GF2EXTENSION
							if (!_is_var_name(term_str[0]))
								term_str[0] = "(" + _int_to_poly_repr(std::stoi(term_str[0])) + ")";
#endif
							
							temp = term_str[0];
							for(unsigned i = 1; i < term_str.size(); ++i)
								temp += "*" + term_str[i];
						}
						s = temp + s;
					}
					polynomial_array.emplace_back(s);
				}

				std::vector<std::string> basis;
#ifdef GF2EXTENSION
				basis = groebnerBasisGF2ExtensionF4(modulus_str, MAXVARS, var_names, gen_name, polynomial_array, this->nrthreads, VERBOSE);
#else
				basis = groebnerBasisF4(FIELDSIZE, MAXVARS, var_names, polynomial_array, this->nrthreads, VERBOSE);
#endif

#ifdef GF2EXTENSION
				for(auto f : basis)
				{
					const std::string delimiter = " + ";
					std::stringstream ss;
					std::vector<std::string> vtermstrs;
					std::size_t pos = 0;
					std::size_t start = 0;
					std::string term_str;

					while((pos = f.find(delimiter, start)) != std::string::npos)
					{
						term_str = f.substr(start+1, pos - start - 2);
						vtermstrs.emplace_back(term_str);
						start += (pos - start) + delimiter.length();
					}
					term_str = f.substr(start+1, f.length() - start - 2);
					vtermstrs.emplace_back(term_str);

					bool first_term = true;
					for(auto termstrs : vtermstrs)
					{
						std::vector<std::string> cvstr;
						std::stringstream _ss;
						sa::split(cvstr, termstrs, '*');

						cvstr[0] = _poly_to_int_repr(cvstr[0]);

						_ss << cvstr[0];
						for(unsigned i=1; i < cvstr.size(); ++i)
							_ss << "*" << cvstr[i];

						if (!first_term) ss << "+";
						ss << _ss.str();

						first_term = false;
					}
					this->solution.emplace_back( this->parser.parse_string(ss.str()) );
				}
#else
				for(auto f : basis)
				{
					sa::erase_pred(f, sa::is_any_of("()"));
					for(unsigned i=0; i < f.length(); ++i)
					{
						if (f[i] == '-')
						{
							std::size_t last = f.find('*', i);
							std::string min_coeff = f.substr(i, last - i);
							
							f =  f.substr(0, i) + std::to_string(FIELDSIZE + stoi(min_coeff)) + f.substr(last, std::string::npos);
							i += last;
						}
					}
					this->solution.emplace_back( this->parser.parse_string(f) );
				}
#endif
			}
	};

	class mysolver_t : public openf4solver<mypolynomial_t>
	{
		public:
			mysolver_t()
			{
				this->_solvername = "openf4";
			}
	};
}

#endif
