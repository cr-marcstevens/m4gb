#include "../lib/polynomial_simple.hpp"
#include "../lib/monomial_degrevlex.hpp"
#include "../lib/gf_p_simple.hpp"

#include "test_core.hpp"

int test()
{
	typedef gb::gf_p_simple<521> gf_t;
	typedef gb::monomial_degrevlex_traits<20,20> traits_t;
	typedef gb::polynomial_simple_t<traits_t, gf_t> polynomial_t;
	typedef typename polynomial_t::monomial_t monomial_t;
	typedef typename monomial_t::pair_t varexp_t;

	typedef std::pair< std::size_t, monomial_t > raw_term_t;
	typedef std::vector< raw_term_t > raw_polynomial_t;

	raw_polynomial_t rawpoly;
	rawpoly.emplace_back(1, monomial_t(varexp_t(1,1)) * monomial_t(varexp_t(2,1)));
	rawpoly.emplace_back(2, monomial_t(varexp_t(2,1)) * monomial_t(varexp_t(3,1)));

	polynomial_t p;
	p = rawpoly;
	CHECK( p.force_test() == 0 );

	polynomial_t p2 = p;
	std::size_t i = 1;
	while (! p2.empty() )
	{
		p2 = p + p2;
		++i;
	}
	CHECK( i == gf_t::gfchar );

	p2 = p * monomial_t(varexp_t(4,1));
	CHECK( p2.force_test() == 0);
	CHECK( p2.size() == p.size() );

	p2 = p2 + p;
	CHECK( p2.force_test() == 0);
	CHECK( p2.size() == 2*p.size() );

	return 0;
}
