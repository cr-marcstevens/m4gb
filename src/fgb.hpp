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

#ifndef M4GB_FGBSOLVER_HPP
#define M4GB_FGBSOLVER_HPP

#include "config.hpp"
#include "../lib/gf_elem_simple.hpp"
#include "../lib/monomial_degrevlex.hpp"
#include "../lib/polynomial_simple.hpp"
#include "../lib/solver_base.hpp"

#include <fgb.h>

#define FGb_MAXI_BASE 100000
#define MAX_MATRIX_ROW 10000000
#define VERBOSE 1

namespace gb
{

	template<typename polynomial_t>
	class fgbsolver : public solver_base_t<polynomial_t>
	{
		public:
			typedef typename polynomial_t::coefficient_t gfelem_t;
			typedef typename polynomial_t::monomial_t monomial_t;
		
		private:
			char *var_names[MAXVARS];

			polynomial_t FGbpoly_to_stdpoly(Dpol f)
			{

				I32 nmono = FGB(nb_terms)(f);
				
				std::vector< std::pair<gfelem_t, monomial_t> > p; p.reserve(nmono);
				I32 *ms = (I32 *) malloc(sizeof(I32) * nmono * MAXVARS);
				I32 *cs = (I32 *) malloc(sizeof(I32) * nmono);

				FGB(export_poly)(MAXVARS, nmono, ms, cs, f);

				for(signed i=0; i < nmono; ++i)
				{
					std::vector< std::pair<I32, I32> > _vm;
					_vm.reserve(MAXVARS);
				
					I32 *ei = ms + i * MAXVARS;
					for(unsigned j = 0; j < MAXVARS; ++j)
					{
						if (ei[j] > 0)
							_vm.emplace_back( std::make_pair(j, ei[j]) );
					}

					p.emplace_back( std::make_pair(gfelem_t(cs[i]), monomial_t(_vm.begin(), _vm.end())) );
				}

				free(ms);
				free(cs);
				return polynomial_t( p.begin(), p.end() );
			}
			
			Dpol stdpoly_to_FGbpoly(const polynomial_t & f)
			{
				Dpol p;

				p = FGB(creat_poly)(f.count());
				unsigned ctr=0;
				for(auto it = f.begin(); it != f.end(); ++it)
				{
					I32 e[MAXVARS] = {};
					
					const gfelem_t & c = it->first;
					const monomial_t & m = it->second;

					for(auto m_it = m.begin(); m_it != m.end(); ++m_it)
					{
						e[m_it->first] = m_it->second;
					}
					FGB(set_expos2)(p, ctr, e, MAXVARS);
					FGB(set_coeff_I32)(p, ctr, c.v);
					++ctr;
				}

				FGB(full_sort_poly2)(p);
				return p;
			}

		public:
			void solve()
			{
				Dpol F[FGb_MAXI_BASE], G[FGb_MAXI_BASE];

				FGB(saveptr)();
				init_FGb_Modp(FIELDSIZE);

				for(int i=0; i < MAXVARS; ++i)
				{
					char var_name[20];
					std::string temp_var(this->parser.var_name(i));

					std::strncpy(var_name, temp_var.c_str(), temp_var.length());
					
					var_names[i] = (char * ) malloc(sizeof(char) * strlen(var_name));
					std::strncpy(var_names[i], var_name, strlen(var_name));
				}

				FGB(PowerSet)(MAXVARS, 0, var_names);

				threads_FGb(this->nrthreads);

				for(unsigned i=0; i < this->input.size(); ++i)
				{
					F[i] = stdpoly_to_FGbpoly(this->input[i]);
				}

				int nb = 0;
				double t = 0;
				SFGB_Options opt;

				FGb_set_default_options(&opt);
				opt._env._force_elim = 0;
				opt._env._index = MAX_MATRIX_ROW;
				opt._verb = VERBOSE;

				nb = FGB(fgb)(F, this->input.size(), G, FGb_MAXI_BASE, &t, &opt);
				
				for(signed i=0; i < nb; ++i)
				{
					this->solution.emplace_back(FGbpoly_to_stdpoly(G[i]));
				}
			
				std::cout << std::endl;
				
				FGB(reset_memory)();
				FGB(restoreptr)();
				for(int i=0; i < MAXVARS; ++i)
					free(var_names[i]);
			}
	};

	class mysolver_t : public fgbsolver<mypolynomial_t>
	{
		public:
			mysolver_t()
			{
				this->_solvername = "fgb";
			}
	};
}

#endif
