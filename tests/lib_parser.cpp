#include "../lib/parser.hpp"
#include "../lib/polynomial_simple.hpp"
#include "../lib/monomial_degrevlex.hpp"
#include "../lib/gf_p_simple.hpp"
#include "../lib/solver_base.hpp"
#include "test_core.hpp"

int test()
{
	typedef gb::gf_p_simple<521> gf_t;
	typedef gb::monomial_degrevlex_traits_uint64<20,20> traits_t;
	typedef gb::polynomial_simple_t<traits_t, gf_t> polynomial_t;
	typedef gb::parser_t<polynomial_t> parser_t;
	typedef gb::dummy_solver_t<polynomial_t> solver_t;

	solver_t solver;
	parser_t parser;
	polynomial_t p = parser.parse_string("1 * x1 * x4+5 * x4 * y3");
	std::cout << p << std::endl;
	return 0;
}

