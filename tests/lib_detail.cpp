#include "../lib/detail.hpp"

#include "test_core.hpp"

int test()
{
	for (size_t i = 0; i < 1000; ++i)
	{
		CHECK(gb::detail::binomial_coefficient(i,0) == 1);
		CHECK(gb::detail::binomial_coefficient(i,1) == i);
		CHECK(gb::detail::binomial_coefficient(i,i) == 1);
		CHECK(gb::detail::binomial_coefficient(i,i+1) == 0);
	}
	CHECK(gb::detail::binomial_coefficient(11,5) == 462);
	CHECK(gb::detail::binomial_coefficient(37,25) == 1852482996);

	for (size_t i = 0; i < 1000; ++i)
	{
		CHECK(gb::detail::multiset_coefficient(i,0) == 1);
		CHECK(gb::detail::multiset_coefficient(i,1) == i);
	}
	CHECK(gb::detail::multiset_coefficient(5,5) == 126);
	CHECK(gb::detail::multiset_coefficient(20,15) == 1855967520);

	CHECK((gb::detail::binomial_coefficient_t<0,0>::value == 1));
	CHECK((gb::detail::binomial_coefficient_t<0,1>::value == 0));
	CHECK((gb::detail::binomial_coefficient_t<1,0>::value == 1));
	CHECK((gb::detail::binomial_coefficient_t<1,1>::value == 1));
	CHECK((gb::detail::binomial_coefficient_t<1,2>::value == 0));
	CHECK((gb::detail::binomial_coefficient_t<2,0>::value == 1));
	CHECK((gb::detail::binomial_coefficient_t<2,1>::value == 2));
	CHECK((gb::detail::binomial_coefficient_t<2,2>::value == 1));
	CHECK((gb::detail::binomial_coefficient_t<2,3>::value == 0));
	CHECK((gb::detail::binomial_coefficient_t<11,5>::value == 462));
	CHECK((gb::detail::binomial_coefficient_t<37,25>::value == 1852482996));

	CHECK((gb::detail::multiset_coefficient_t<0,0>::value == 1));
	CHECK((gb::detail::multiset_coefficient_t<0,1>::value == 0));
	CHECK((gb::detail::multiset_coefficient_t<1,0>::value == 1));
	CHECK((gb::detail::multiset_coefficient_t<1,1>::value == 1));
	CHECK((gb::detail::multiset_coefficient_t<1,2>::value == 1));
	CHECK((gb::detail::multiset_coefficient_t<2,0>::value == 1));
	CHECK((gb::detail::multiset_coefficient_t<2,1>::value == 2));
	CHECK((gb::detail::multiset_coefficient_t<2,2>::value == 3));
	CHECK((gb::detail::multiset_coefficient_t<2,3>::value == 4));
	CHECK((gb::detail::multiset_coefficient_t<5,5>::value == 126));
	CHECK((gb::detail::multiset_coefficient_t<20,15>::value == 1855967520));

	CHECK((gb::detail::nrbits_t<1>::value == 1));
	CHECK((gb::detail::nrbits_t<2>::value == 2));
	CHECK((gb::detail::nrbits_t<3>::value == 2));
	CHECK((gb::detail::nrbits_t<4>::value == 3));
	CHECK((gb::detail::nrbits_t<5>::value == 3));
	CHECK((gb::detail::nrbits_t<2305843009213693952ULL>::value == 62));

	CHECK((gb::detail::nrbytes_t<255>::value == 1));
	CHECK((gb::detail::nrbytes_t<256>::value == 2));
	CHECK((gb::detail::nrbytes_t<65535>::value == 2));
	CHECK((gb::detail::nrbytes_t<65536>::value == 3));
	CHECK((gb::detail::nrbytes_t<16777215>::value == 3));
	CHECK((gb::detail::nrbytes_t<16777216>::value == 4));

	CHECK(( sizeof(gb::detail::least_unsigned_integer_t<1>::type) == 1));
	CHECK(( sizeof(gb::detail::least_unsigned_integer_t<2>::type) == 2));
	CHECK(( sizeof(gb::detail::least_unsigned_integer_t<3>::type) >= 3));
	CHECK(( sizeof(gb::detail::least_unsigned_integer_t<4>::type) == 4));
	CHECK(( sizeof(gb::detail::least_unsigned_integer_t<5>::type) >= 5));
	CHECK(( sizeof(gb::detail::least_unsigned_integer_t<6>::type) >= 6));
	CHECK(( sizeof(gb::detail::least_unsigned_integer_t<7>::type) >= 7));
	CHECK(( sizeof(gb::detail::least_unsigned_integer_t<8>::type) == 8));

	CHECK(( sizeof(gb::detail::least_nonchar_unsigned_integer_t<1>::type) == 2));
	CHECK(( sizeof(gb::detail::least_nonchar_unsigned_integer_t<2>::type) == 2));
	CHECK(( sizeof(gb::detail::least_nonchar_unsigned_integer_t<3>::type) >= 3));
	CHECK(( sizeof(gb::detail::least_nonchar_unsigned_integer_t<4>::type) == 4));
	CHECK(( sizeof(gb::detail::least_nonchar_unsigned_integer_t<5>::type) >= 5));
	CHECK(( sizeof(gb::detail::least_nonchar_unsigned_integer_t<6>::type) >= 6));
	CHECK(( sizeof(gb::detail::least_nonchar_unsigned_integer_t<7>::type) >= 7));
	CHECK(( sizeof(gb::detail::least_nonchar_unsigned_integer_t<8>::type) == 8));

	CHECK(gb::detail::factor_int(2) == std::vector<std::size_t>(1,2));
	CHECK(gb::detail::factor_int(5) == std::vector<std::size_t>(1,5));
	CHECK(gb::detail::factor_int(521) == std::vector<std::size_t>(1,521));

	std::vector<std::size_t> v1 = gb::detail::factor_int(521*601*2);
	CHECK(v1.size() == 3 && v1[0] == 2 && v1[1] == 521 && v1[2] == 601);
	
	return 0;
}

