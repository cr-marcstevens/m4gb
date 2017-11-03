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

#ifndef M4GB_SOLVER_COMMON_HPP
#define M4GB_SOLVER_COMMON_HPP

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "config.hpp"

#ifdef PRINT_PROCESS_STATISTICS
#include "../contrib/runtime_stats.hpp"
#endif

namespace po = boost::program_options;

template<typename ordering_tag>
struct ordering_name { };

template<>
struct ordering_name<gb::degrevlex_tag_t>
{ static std::string name() { return "degrevlex"; } };

int main(int argc, char** argv)
{
	const std::size_t fieldsize = gb::mysolver_t::fieldsize;
	const std::size_t fieldchar = gb::mysolver_t::fieldchar;
	const std::size_t max_vars = gb::mysolver_t::max_vars;
#ifndef MQ_NO_TRY_CATCH
	try 
	{
#endif
		std::string inputfile, outputfile;
		std::vector<std::string> options;
		unsigned loglevel;
		unsigned nrthreads;

		po::options_description
			opt_cmds("Allowed commands"),
			opt_opts("Allowed options"),
			all("Allowed options");

		opt_cmds.add_options()
			("help,h", "Show options\n")
			("fieldsize", "Return Galois Field size")
			("fieldchar", "Return Galois Field characteristic")
			("maxvars", "Return maximum number of variables")
			("solvername", "Returns solver name")
			("ordering", "Return monomial ordering")
			("solve,s", "Solve input system of equations")
			("showinput", "Print input system")
			("showoutput", "Print output system")
			;
		opt_opts.add_options()
			("inputfile,i", po::value<std::string>(&inputfile), "Input file")
			("outputfile,o", po::value<std::string>(&outputfile), "Output file")
			("mqchallenge", "Read inputfile in mqchallenge format")
			("default", "Read inputfile in default format")
			("loglevel", po::value<unsigned>(&loglevel)->default_value( (int)(gb::lg_info) ), "Set log level:\n\t0=abort, 1=error, 2=warning, 3=info, 4-7=verbose")
			("nrthreads", po::value<unsigned>(&nrthreads)->default_value(boost::thread::hardware_concurrency()), "Maximum number of threads to use")
			;
		all.add(opt_cmds).add(opt_opts);

		po::variables_map vm;
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(all).allow_unregistered().run();
		po::store(parsed, vm);
		po::notify(vm);

		for (auto opt : collect_unrecognized(parsed.options, po::include_positional))
		{
			std::size_t pos = opt.find_first_not_of('-');
			opt.erase(0, pos);
			if (opt.empty())
				continue;
			gb::get_options().set(opt);
			std::cout << "Passing module option: " << opt << std::endl;
		}

		gb::get_logger().set_log_level((gb::log_level)(loglevel));

		if (vm.count("fieldsize"))
		{
			std::cout << fieldsize << std::endl;
			return 0;
		}
		if (vm.count("fieldchar"))
		{
			std::cout << fieldchar << std::endl;
			return 0;
		}
		if (vm.count("maxvars"))
		{
			std::cout << max_vars << std::endl;
			return 0;
		}
		if (vm.count("solvername"))
		{
			std::cout << SOLVERNAME << std::endl;
			return 0;
		}
		if (vm.count("ordering"))
		{
			std::cout << ordering_name<gb::mysolver_t::monomial_t::order_tag_t>::name() << std::endl;
			return 0;
		}
		if (vm.count("help") || vm.count("solve")+vm.count("showinput") == 0 || inputfile.empty())
		{
			std::cout << opt_cmds << opt_opts << std::endl;
			return 0;
		}

		gb::mysolver_t solver;
		solver.nrthreads = nrthreads;

		if (vm.count("mqchallenge"))
			solver.read_file(inputfile, gb::pf_mqchallenge);
		else
			solver.read_file(inputfile, gb::pf_default);

		solver.solve();

		if (vm.count("showoutput") || outputfile.empty())
		{
			solver.print_solution();
		}
		if (!outputfile.empty())
		{
			solver.save_solution(outputfile);
		}
#ifndef MQ_NO_TRY_CATCH
	}
	catch (std::exception& e)
	{
		std::cerr << "Caught exception:" << std::endl << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << "Caught unknown exception!" << std::endl;
		return 1;
	}
#endif

#ifdef PRINT_PROCESS_STATISTICS
	std::cout << "Total CPU time : " << getCPUTime() << " s" << std::endl;
	std::cout << "Peak memory    : " << (getPeakRSS() >> 20) << " MiB" << std::endl;
	std::cout << "Resource index : " << double(getCPUTime())*double(getPeakRSS()>>20) << " s*MiB" << std::endl;
#endif

	return 0;
}

#endif