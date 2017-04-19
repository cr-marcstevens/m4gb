#include "../lib/monomial_degrevlex.hpp"

#include "test_core.hpp"

#include <algorithm>

int test()
{
	typedef gb::monomial_degrevlex_traits_uint64<20,16> monomial_traits;
	typedef typename monomial_traits::int_monomial_t int_monomial_t;
	typedef typename monomial_traits::static_monomial_t static_monomial_t;
	typedef typename monomial_traits::dynamic_monomial_t dynamic_monomial_t;

	int_monomial_t im;
	static_monomial_t sm;
	dynamic_monomial_t dm;

	std::vector<dynamic_monomial_t> vec;
	for (std::size_t i = 0; i < (1<<20); ++i)
	{
		vec.emplace_back(int_monomial_t(i));
		CHECK( (static_monomial_t)(int_monomial_t(i)) == int_monomial_t(i) );
		CHECK( (dynamic_monomial_t)(int_monomial_t(i)) == int_monomial_t(i) );
		CHECK( (static_monomial_t)(int_monomial_t(i)) == (dynamic_monomial_t)(int_monomial_t(i)) );
	}
	CHECK( vec.back().degree() == 8 );

	for (std::size_t i = 0; i < vec.size() && i < (1<<9); ++i)
		for (std::size_t j = 0; j < vec.size() && j < (1<<9); ++j)
		{
			CHECK( (vec[i] == vec[j]) == (i==j) );
			CHECK( (vec[i] != vec[j]) == (i!=j) );
			CHECK( (vec[i] < vec[j]) == (i<j) );
			CHECK( (vec[i] > vec[j]) == (i>j) );
			CHECK( (vec[i] <= vec[j]) == (i<=j) );
			CHECK( (vec[i] >= vec[j]) == (i>=j) );
			CHECK( (vec[i] == vec[j]) == (i==j) );
			CHECK( (vec[i] != vec[j]) == (i!=j) );
			CHECK( ((static_monomial_t)(vec[i]) == vec[j]) == (i==j) );
			CHECK( ((static_monomial_t)(vec[i]) != vec[j]) == (i!=j) );
			CHECK( ((static_monomial_t)(vec[i]) < vec[j]) == (i<j) );
			CHECK( ((static_monomial_t)(vec[i]) > vec[j]) == (i>j) );
			CHECK( ((static_monomial_t)(vec[i]) <= vec[j]) == (i<=j) );
			CHECK( ((static_monomial_t)(vec[i]) >= vec[j]) == (i>=j) );
			CHECK( ((static_monomial_t)(vec[i]) == vec[j]) == (i==j) );
			CHECK( ((static_monomial_t)(vec[i]) != vec[j]) == (i!=j) );
			CHECK( ((int_monomial_t)(vec[i]) == j) == (i == j) );
			auto tmp = vec[i] * vec[j];
			CHECK( (tmp/vec[j]) == vec[i] );
			CHECK( tmp >= vec[i] );
			CHECK( tmp >= vec[j] );
		}

	return 0;
}

