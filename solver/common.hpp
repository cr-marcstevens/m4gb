#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#define PRINT_PROCESS_STATISTICS


#ifndef MAXVARS
#define MAXVARS   20
#endif

#ifndef FIELDSIZE
#define FIELDSIZE 31
#endif

#ifndef SOLVERNAME
#define SOLVERNAME "m4gb"
#endif

#ifndef SOLVERHPP
#define SOLVERHPP "m4gb.hpp"
#endif 

#if FIELDSIZE <= 256
#define GF_USE_MUL_TABLE
#endif

#if FIELDSIZE <= 64
#define GF_USE_ADD_MUL_TABLE
#endif

#if FIELDSIZE == 2
#define MONOMIAL_ALLOW_FIELDEQUATIONS
#define GB_ADD_FIELDEQUATIONS
#define MONOMIAL_NO_WRAPCHECK
#endif

#if FIELDSIZE >= 31
#define MONOMIAL_NO_WRAPCHECK
#endif

#define MQ_NO_TRY_CATCH

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
		unsigned loglevel;

		gb::mysolver_t solver;

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
			("nrthreads", po::value<unsigned>(&solver.nrthreads)->default_value(boost::thread::hardware_concurrency()), "Maximum number of threads to use")
			;
		all.add(opt_cmds).add(opt_opts);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(all).run(), vm);
		po::notify(vm);

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
		if (vm.count("help") || vm.count("solve") == 0 || inputfile.empty())
		{
			std::cout << opt_cmds << opt_opts << std::endl;
			return 0;
		}

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
	std::cout << "CPU time  : " << getCPUTime() << "s" << std::endl;
	std::cout << "Max memory: " << (getPeakRSS() >> 20) << "MiB" << std::endl;
#endif

	return 0;
}

