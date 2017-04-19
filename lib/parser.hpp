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

#ifndef M4GB_PARSER_HPP
#define M4GB_PARSER_HPP

#include "detail.hpp"
#include "monomial_degrevlex.hpp"
#include "logger.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include <cctype>
#include <cstdlib>

namespace gb {

	enum poly_format { pf_default, pf_mqchallenge };

	template<typename Poly>
	class parser_t
	{
	public:
		typedef Poly polynomial_t;
		typedef typename polynomial_t::field_t field_t;
		typedef typename polynomial_t::monomial_t monomial_t;
		typedef typename polynomial_t::coefficient_t coefficient_t;
		typedef typename polynomial_t::term_t  term_t;

		static const std::size_t max_vars = polynomial_t::max_vars;
		static const std::size_t fieldsize = polynomial_t::fieldsize;
		static const std::size_t fieldchar = polynomial_t::fieldchar;

		std::vector<polynomial_t> polynomials;

	private:
		std::vector<std::string> _var_names;
		logger_t& _logger;
		uint64_t str_to_uint64(const std::string& str) { return strtoull(str.c_str(), NULL, 0); }
	public:

		parser_t()
			: _logger(get_logger())
		{
		}

		void clear()
		{
			polynomials.clear();
			_var_names.clear();
		}

		void add_var_name(const std::string& varname)
		{
			if (_var_names.size() >= max_vars)
				_log(lg_warn) << "add_var_name(): limit of " << max_vars << " reached: discarded '" << varname << "'" << std::endl;
			else
			{
				if (std::find(_var_names.begin(), _var_names.end(), varname) != _var_names.end())
					throw std::runtime_error("duplicate variable found.");
				_var_names.push_back(varname);
				_log(lg_verbose) << "added variable name '" << varname << "'" << std::endl;
			}
		}

		// call before any parsing!: clears polynomials to prevent any unwanted variable index collisions
		void set_var_names(const std::vector<std::string>& varnames)
		{
			polynomials.clear();
			_var_names.clear();
			for (auto vn : varnames)
				add_var_name(vn);
		}

		std::string var_name(unsigned i)
		{
			if (i >= max_vars)
				return std::string();
			if (i >= _var_names.size())
			{
				std::string ret = std::string("x") + std::to_string(i);
				if (std::find(_var_names.begin(), _var_names.end(), ret) != _var_names.end())
					_log(lg_abort) << "variable name '" << ret << "' has two indexes: " << (std::find(_var_names.begin(), _var_names.end(), ret)-_var_names.begin()) << " " << i << std::endl;
				return ret;
			}
			return _var_names[i];
		}

		void read_file_default(const std::string& path)
		{
			clear();
			_parse_default_format(path);
		}

		void read_file_mqchallenge(const std::string& path)
		{
			clear();
			_parse_mqchallenge_format(path);
		}

		void read_file(const std::string& path, poly_format pf)
		{
			switch (pf)
			{
			case pf_default:
				read_file_default(path);
				break;
			case pf_mqchallenge:
				read_file_mqchallenge(path);
				break;
			default:
				throw std::runtime_error("parser::parse_file(path,poly_format): unknown format!");
			}
		}


		polynomial_t parse_coefficients_degrevlex(const std::vector<coefficient_t>& coefficients, unsigned nrvars = max_vars)
		{
			if (nrvars > max_vars)
				throw std::runtime_error("parser::parse_polynomial(coefficients,nrvars): nrvars > max_vars");

			typedef typename monomial_degrevlex_traits<monomial_t::max_vars, monomial_t::exponent_order>::monomial_t monomial_degrevlex_t;
			std::vector<monomial_degrevlex_t> monomials = _generate_monomials<monomial_degrevlex_t>(coefficients.size(), nrvars);

			std::vector<term_t> terms;
			std::size_t j = coefficients.size() - 1;
			for (std::size_t i = 0; i < coefficients.size(); ++i, --j)
				if (coefficients[i] != 0)
					terms.emplace_back(coefficients[i], monomials[j]);

			return polynomial_t(terms.begin(), terms.end());
		}

		polynomial_t& addpoly_coefficients_degrevlex(const std::vector<coefficient_t>& coefficients, unsigned nrvars = max_vars)
		{
			polynomials.emplace_back(parse_coefficients_degrevlex(coefficients, nrvars));
			_log(lg_verbose) << "added: " << polynomial_to_string(polynomials.back()) << std::endl;
			return polynomials.back();
		}

		polynomial_t parse_string(const std::string& str)
		{
			// obtain only [+, 0-9, a-z, A-Z, ^, _,  ], replace '*' by ' ' and '  ' by ' '
			std::string polynomial_str;
			for (auto c : str)
			{
				if (c == '+' || c == '-' || isalnum(c) || c == '^' || c == '_' || c == ' ')
					polynomial_str.push_back( std::tolower(c) );
				else
					if (c == '*')
						polynomial_str.push_back(' ');
			}
			// removes space around ^ to ensure "varname^exponent" has no spaces
			std::size_t pos = polynomial_str.find("  ");
			while (pos != std::string::npos)
			{
				polynomial_str.erase(pos+1, 1);
				pos = polynomial_str.find("  ");
			}
			pos = polynomial_str.find(" ^");
			while (pos != std::string::npos)
			{
				polynomial_str.erase(pos, 1);
				pos = polynomial_str.find(" ^");
			}
			pos = polynomial_str.find("^ ");
			while (pos != std::string::npos)
			{
				polynomial_str.erase(pos+1, 1);
				pos = polynomial_str.find("^ ");
			}

			//insert "+" before the "-".
			pos = polynomial_str.find("-");
			while (pos != std::string::npos)
			{
				polynomial_str.insert(pos, "+");
				pos = polynomial_str.find("-", pos+2);
			}

			_log(lg_verbose4) << "polynomial_str = '" << polynomial_str << "'" << std::endl;

			std::vector<std::string> term_strs = _split(polynomial_str, '+');

			std::vector<term_t> terms;
			for (auto termstr : term_strs)
			{
				std::string tmpstr = _trim(termstr);
				bool negative = false;
				if (!tmpstr.empty() && tmpstr[0] == '-')
				{
					negative = true;
					tmpstr.erase(0, 1);
					tmpstr = _trim(tmpstr);
				}
				if (tmpstr.empty())
					continue;
				_log(lg_verbose4) << "   term_str = '" << tmpstr << "'" << std::endl;

				term_t term;
				term.second.clear();
				if (negative)
					term.first = -coefficient_t(1);
				else
					term.first = coefficient_t(1);

				std::vector<std::string> parts = _split(tmpstr, ' ');

				std::map<uint64_t, uint64_t> monomial_v;
				for (auto part : parts)
				{
					std::string partstr = _trim(part);
					if (partstr.empty())
						continue;
					_log(lg_verbose4) << "      part_str = '" << partstr << "'" << std::endl;
					if (std::isdigit(partstr[0]))
					{
						uint64_t cof = str_to_uint64(partstr);
						term.first *= (coefficient_t)(cof);
						_log(lg_verbose4) << "      coefficient = " << term.first << std::endl;
					} 
					else if (std::isalpha(partstr[0]))
					{
						uint64_t e = 1;
						std::size_t exppos = partstr.find('^');
						if (exppos != std::string::npos)
						{
							e = str_to_uint64(_trim(partstr.substr(exppos + 1)));
							partstr = _trim(partstr.substr(0, exppos));
							_log(lg_verbose4) << "      exponent = " << e << std::endl;
						}
						auto it = std::find(_var_names.begin(), _var_names.end(), partstr);
						if (it == _var_names.end())
						{
							if (_var_names.size() >= max_vars)
							{
								_log(lg_error) << "parse_string(): too many variable names" << std::endl;
								return polynomial_t();
							}
							add_var_name(partstr);
							it = _var_names.end() - 1;
						}
						uint64_t i = it - _var_names.begin();
						_log(lg_verbose4) << "      varname = '" << partstr << "', varidx = " << i << std::endl;
						monomial_v[i] += e;
					}
				}
				term.second.assign(monomial_v.begin(), monomial_v.end());
				_log(lg_verbose4) << "   term = " << term.second << std::endl;
				if (term.first != 0)
					terms.emplace_back(std::move(term));
			}
			polynomial_t polynomial(terms.begin(), terms.end());
			return polynomial;
		}

		polynomial_t& addpoly_string(const std::string& str)
		{
			polynomials.emplace_back(parse_string(str));
			_log(lg_verbose) << "added: " << polynomial_to_string(polynomials.back()) << std::endl;
			return polynomials.back();
		}

		std::string polynomial_to_string(const polynomial_t& poly)
		{
			std::vector<std::string> varnames;
			for (unsigned i = 0; i < max_vars; ++i)
				varnames.emplace_back(var_name(i));

			std::stringstream o;
			bool firstterm = true;
			for (auto term : poly)
			{
				if (term.first == 0)
					continue;

				if (firstterm)
					firstterm = false;
				else
					o << " + ";

				if (term.second.empty())
					o << term.first;
				else
				{
					if (term.first != 1)
						o << term.first << "*";
					bool firstvar = true;
					for (auto varsit = term.second.begin(); varsit != term.second.end(); ++varsit)
					{
						if (firstvar)
							firstvar = false;
						else
							o << "*";
						if (varsit->second == 1)
							o << varnames[varsit->first];
						else if (varsit->second > 1)
							o << varnames[varsit->first] << "^" << (std::size_t)(varsit->second);
					}
				}
			}
			if (firstterm)
				o << "0";
			return o.str();
		}

	private:
		logger_t::logger_stream_t _log(log_level ll)
		{
			return _logger("parser", ll);
		}

		std::string _trim(const std::string& str)
		{
			std::string tmp = str;
			while (!tmp.empty() && std::isspace(tmp.back()))
				tmp.pop_back();
			while (!tmp.empty() && std::isspace(tmp[0]))
				tmp.erase(0, 1);
			return tmp;
		}

		std::vector<std::string> _split(const std::string& str, char delim)
		{
			std::vector<std::string> ret;
			std::size_t oldpos = 0, pos = str.find(delim);
			while (pos != std::string::npos)
			{
				ret.emplace_back(str.substr(oldpos, pos - oldpos));
				oldpos = pos+1;
				pos = str.find(delim, pos + 1);
			}
			ret.emplace_back(str.substr(oldpos));
			return ret;
		}

		void _parse_default_format(const std::string& path)
		{
			_log(lg_info) << "loading polynomials from '" << path << "' (format: default)" << std::endl;
			std::ifstream ifs(path);
			if (!ifs)
			{
				_log(lg_error) << "Failed to open file '" << path << "'" << std::endl;
				return;
			}
			std::string line;
			while (getline(ifs, line))
			{
				// ignore everything after a #
				std::size_t pos = line.find('#');
				if (pos < line.size())
					line.resize(pos);
				line = _trim(line);
				// disregard resulting empty lines
				if (line.empty())
					continue;

				if (line[0] == '$')
				{
					std::vector<std::string> line_substr = _split(line, ' ');
					if (line_substr[0] == "$fieldsize")
					{
						if (fieldsize != std::stoi(line_substr[1]))
							throw std::runtime_error("field size mismatch.");
					}
					else if (line_substr[0] == "$vars")
					{
						if (std::isdigit(line_substr[1][0]))
						{
							std::size_t nvars = std::stoi(line_substr[1]);
							std::string var_prefix = line_substr[2];

							for(std::size_t i=0; i < nvars; ++i)
								add_var_name(var_prefix + std::to_string(i));
						}
						else if (std::isalpha(line_substr[1][0]))
						{
							for(std::size_t i = 1; i < line_substr.size(); ++i)
								add_var_name(line_substr[i]);
						}
					}
					else
					{
						throw std::runtime_error("command is not defined.");
					}

				}
				else
				{
					addpoly_string(line);
				}
			}
		}

		void _parse_mqchallenge_format(const std::string& path)
		{
			_log(lg_info) << "loading polynomials from '" << path << "' (format: mqchallenge)" << std::endl;
			std::ifstream ifs(path);
			if (!ifs)
			{
				_log(lg_error) << "Failed to open file '" << path << "'" << std::endl;
				return;
			}

			uint64_t m, seed;
			std::string monomial_order;
			unsigned nrvars = 0;

			std::string line;
			const std::string delimiter = ":";

			while (getline(ifs, line))
			{
				line = _trim(line);
				if (line.empty())
					continue;
				if (line.find("******************") != std::string::npos)
					break;

				std::size_t pos = line.find(delimiter);
				if (pos >= line.size())
					throw;
				std::string left = _trim(line.substr(0, pos));
				std::string right = _trim(line.substr(pos + delimiter.length(), line.size() - pos));

				if (left == "Galois Field")
				{
					unsigned gfchar, modulo = 0;

					if (right.find("/") == std::string::npos)
					{
						if (right.substr(0, 3) == "GF(")
						{
							gfchar = stoi(right.substr(3, right.find(')') - 3));
							_log(lg_verbose) << "mqchallenge: GF(" << gfchar << ")" << std::endl;
							if (fieldsize != gfchar)
							{
								_log(lg_error) << "mqchallenge: field size mismatch" << std::endl;
								return;
							}
						}
					}
					else
					{
						unsigned deg_of_ext = 0;
						right.erase(remove_if(right.begin(), right.end(), isspace), right.end());
						//boost::erase_all(right, " ");
						gfchar = stoi(right.substr(3, right.find(')') - 3));
						std::string modulo_str = right.substr(right.find('/') + 1);

						//parse modulo_str
						std::vector<std::string> v = _split(modulo_str, '+');
						for (auto s : v)
						{
							std::string ss = _trim(s);
							if (ss == "1")
							{
								modulo |= 1;
							}
							else
							{
								unsigned d = 1;
								if (ss.substr(0, 2) == "x^")
								{
									d = stoi(ss.substr(2));
									modulo |= (1 << d);
									if (d > deg_of_ext)
										deg_of_ext = d;
								}
							}
						}
						_log(lg_verbose) << "mqchallenge: GF(" << gfchar << "^" << deg_of_ext << ")" << std::endl;

						if (fieldchar != gfchar)
						{
							_log(lg_error) << "mqchallenge: field characteristic mismatch:" << fieldchar << " != " << gfchar << std::endl;
							return;
						}
						if (fieldsize != std::size_t(1) << deg_of_ext)
						{
							_log(lg_error) << "mqchallenge: field size mismatch" << std::endl;
							return;
						}
					}
				}
				else if (left == "Number of variables (n)")
				{
					nrvars = std::stoi(right);
					_log(lg_verbose) << "mqchallenge: nrvars = " << nrvars << std::endl;
					if (nrvars > max_vars)
					{
						_log(lg_error) << "mqchallenge: nrvars > max_vars" << std::endl;
						return;
					}
				}
				else if (left == "Number of polynomials (m)")
				{
					m = std::stoi(right);
					_log(lg_verbose) << "mqchallenge: m = " << m << std::endl;
				}
				else if (left == "Seed")
				{
					seed = std::stoi(right);
					_log(lg_verbose) << "mqchallenge: seed = " << seed << std::endl;
				}
				else if (left == "Order")
				{
					monomial_order = right;
					_log(lg_verbose) << "mqchallenge: mono-order = " << monomial_order << std::endl;
				}
			}

			while (getline(ifs, line))
			{
				if (!line.empty() && line[0] == '#')
					continue;
				std::stringstream ss;
				std::size_t temp;
				std::vector<coefficient_t> coefficients;

				ss << line;

				if ((fieldchar == 2) && (fieldsize > 2))
				{
					while (ss >> std::hex >> temp)
					{
						coefficient_t n(temp);
						coefficients.emplace_back(n);
					}
				}
				else
				{
					while (ss >> temp)
					{
						coefficient_t n(temp % fieldsize);
						coefficients.emplace_back(n);
					}
				}

				if (monomial_order == "graded reverse lex order")
					addpoly_coefficients_degrevlex(coefficients, nrvars);
				else
				{
					_log(lg_error) << "mqchallenge: unknown monomial order" << std::endl;
					return;
				}
			}
		}

		// generate sorted vector of monomials up to and including degree D, such that # monomials >= size
		template<typename Mono>
		std::vector<Mono> _generate_monomials(std::size_t size, unsigned nrvars)
		{
			std::vector<Mono> monomials(1);

			unsigned deg = 0;
			while (monomials.size() < size)
			{
				++deg;
				std::vector<std::pair<unsigned, unsigned> > exponents(nrvars);
				for (unsigned i = 0; i < nrvars; ++i)
					exponents[i].first = i;
				exponents[nrvars - 1].second = deg;
				while (true)
				{
					monomials.emplace_back(Mono(exponents.begin(), exponents.end()));

					if (exponents[nrvars - 1].second != 0)
					{
						++exponents[nrvars - 2].second;
						--exponents[nrvars - 1].second;
					}
					else
					{
						int i = nrvars - 2;
						while (i >= 0 && exponents[i].second == 0)
							--i;
						if (i <= 0)
							break;
						++exponents[i - 1].second;
						for (; i < (int)(nrvars - 1); ++i)
						{
							exponents[nrvars - 1].second += exponents[i].second;
							exponents[i].second = 0;
						}
						--exponents[nrvars - 1].second;
					}
				}
			}
			std::sort(monomials.begin(), monomials.end());
			return monomials;
		}
	};

	template<typename Poly> const std::size_t parser_t<Poly>::max_vars;
	template<typename Poly>	const std::size_t parser_t<Poly>::fieldchar;

} // namespace gb

#endif // M4GB_PARSER_HPP
