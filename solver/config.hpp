#include "../lib/config.h"

#define PRINT_PROCESS_STATISTICS

#ifndef MAXVARS
#define MAXVARS   20
#endif

#ifndef FIELDSIZE
#define FIELDSIZE 31
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

#include "../lib/gf_p_simple.hpp"
#include "../lib/gf_2n_simple.hpp"
#include "../lib/monomial_degrevlex.hpp"
#include "../lib/polynomial_simple.hpp"
#include "../lib/polynomial_int.hpp"

namespace gb
{
	namespace detail 
	{
		template<std::size_t fieldsize, std::size_t parity>
		struct getfield
		{
			static_assert( is_prime<fieldsize>::value, "An odd fieldsize must be a prime");
			typedef ::gb::gf_p_simple<fieldsize> type;
		};

		template<std::size_t fieldsize>
		struct getfield<fieldsize,0>
		{
			static const std::size_t extension = nrbits_t<fieldsize>::value - 1;
			static_assert( fieldsize == std::size_t(1) << extension, "An even fieldsize must be a power of 2" );
			typedef ::gb::gf_2n_simple<extension> type;
		};

		template<>
		struct getfield<2,0>
		{
			typedef ::gb::gf_2n_simple<1, 2> type;
		};
	}

	template<std::size_t fieldsize>
	struct getfield
	{
		typedef typename detail::getfield<fieldsize,fieldsize&1>::type type; 
	};

	typedef getfield<FIELDSIZE>::type myfield_t;
	typedef polynomial_simple_t<monomial_degrevlex_traits<MAXVARS, FIELDSIZE>, myfield_t> mypolynomial_t;
}
