#include "config.hpp"

#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

typedef gb::myfield_t field_t;
typedef field_t::gfelm_t coefficient_t;
typedef gb::monomial_degrevlex_traits_uint32<MAXVARS> monomial_int_traits_t;
static_assert(monomial_int_traits_t::max_deg >= DEG, "Number of monomials too large >= 2^32")

typedef monomial_int_traits_t::int_monomial_t monomial_int_t;
typedef monomial_int_traits_t::static_monomial_t monomial_static_t;

typedef gb::polynomial_simple_t<monomial_int_traits_t, field_t> polynomial_t;

const std::size_t NMONOMIALS = gb::detail::multiset_coefficient_t<MAXVARS + 1, DEG>::value;

int main(int argc, char **argv)
{
	std::string outputname;
	unsigned nrequations = 0;
	bool forcesolution = false;
	std::random_device rd;
	std::uint32_t seed = rd();

	po::options_description
		opt_cmds("Allowed commands"),
		opt_opts("Allowed options"),
		all("Allowed options")
		;

	opt_cmds.add_options()
		("help,h", "Show options\n")
		;

	opt_opts.add_options()
		("nrequations,m", po::value<unsigned>(&nrequations), "Number of equations")
		("outputfile,o", po::value<std::string>(&outputname), "Output file")
		("forceroot,r", po::bool_switch(&forcesolution), "Force one root for system")
		("seed", po::value<std::uint32_t>(&seed), "Set pseudo random generator seed")
		;
	all.add(opt_cmds).add(opt_opts);
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(all).run(), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << opt_cmds << opt_opts << std::endl;
		return 0;
	}
	if (vm.count("nrequations") == 0)
	{
		std::cout << "Number of equations must be given with" << std::endl;
		return 0;
	}
	if (vm.count("outputfile") == 0)
	{
		outputname = std::to_string(FIELDSIZE) + "_" + "n"
			+ std::to_string(MAXVARS) + "_" + "m"
			+ std::to_string(nrequations);
	}

	std::array<monomial_static_t, NMONOMIALS> monomials;
	std::vector<std::array<coefficient_t, NMONOMIALS>> coeff_matrix(nrequations);
	std::array<coefficient_t, MAXVARS> solution;

	/* generate all monomials */
	for (std::size_t i = 0; i < NMONOMIALS; ++i)
		monomials[i] = monomial_int_t(i);

	/* random number generator */
	std::mt19937 gen(seed);
	std::uniform_int_distribution<> dist(0, FIELDSIZE - 1);

	/* generate random coefficients */
	for (auto & row : coeff_matrix)
		for (auto & c : row)
			c = dist(gen);

	/* generate solution */
	for (auto & s : solution)
		s = dist(gen);

	/* substitute polynomials */
	if (forcesolution)
	{
		for (auto & row : coeff_matrix)
		{
			coefficient_t sum_val = 0;
			for (std::size_t idx = 0; idx < NMONOMIALS; ++idx)
			{
				if (row[idx] == 0)
					continue;

				coefficient_t term_val = row[idx];
				const monomial_static_t & m = monomials[idx];

				for (const auto & ie : m)
				{
					unsigned i = ie.first;
					unsigned e = ie.second;
					do
					{
						term_val *= solution[i];
						--e;
					} while (e);
				}
				sum_val += term_val;
			}

			/* adjust constant term */
			row[0] -= sum_val;
		}
	}

	/* print file */
	std::string infilename = outputname + ".in";
	std::string ansfilename = outputname + ".ans";


	/* write input file */
	{
		std::cout << std::endl;
		std::cout << "Input file               : " << infilename << std::endl;
		std::ofstream in_os(infilename);
		in_os << "$fieldsize " << std::to_string(FIELDSIZE) << std::endl;
		in_os << "$vars " << std::to_string(MAXVARS) << " X" << std::endl;
		for (const auto & row : coeff_matrix)
		{
			std::vector<std::pair<coefficient_t, monomial_static_t>> v;
			for (std::size_t i = 0; i < NMONOMIALS; ++i)
			{
				if (row[i] == 0)
					continue;
				v.emplace_back(std::make_pair(row[i], monomials[i]));
			}
			polynomial_t f(v.begin(), v.end());
			in_os << f << std::endl;
		}
	}

	/* write answer file */
	if (forcesolution)
	{
		std::cout << "Answer file              : " << ansfilename << std::endl;
		std::ofstream ans_os(ansfilename);
		std::vector<polynomial_t> gb(nrequations);

		std::reverse(solution.begin(), solution.end());
		for (std::size_t i = 1; i <= MAXVARS; ++i)
		{
			std::vector<std::pair<coefficient_t, monomial_static_t>> v;
			v.emplace_back(std::make_pair(1, monomials[i]));
			v.emplace_back(std::make_pair(-solution[i - 1], monomials[0]));
			polynomial_t f(v.begin(), v.end());
			ans_os << f << std::endl;
		}
	}

	return 0;
}
