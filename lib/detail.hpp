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

#ifndef M4GB_DETAIL_HPP
#define M4GB_DETAIL_HPP

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <iostream>

namespace gb
{

	namespace detail
	{

		template<typename UInt = std::size_t>
		UInt binomial_coefficient(std::size_t N, std::size_t K)
		{
			static std::vector< std::vector< std::pair<UInt,bool> > > table(1, std::vector<std::pair<UInt,bool> >(1, std::make_pair<UInt,bool>(UInt(1),false)));
			if (K > N)
				return UInt(0);
			if (N >= table.size())
			{
				table.reserve(N+1);
				for (std::size_t i = table.size(); i <= N; ++i)
				{
					table.emplace_back(i+1, std::make_pair<UInt,bool>(UInt(0),false));
					table[i][0].first = table[i][i].first = UInt(1);
					for (std::size_t j = 1; j < i; ++j)
					{
						table[i][j].first = table[i-1][j-1].first + table[i-1][j].first;
						// determine  if an overflow happened somewhere
						table[i][j].second = table[i-1][j-1].second | table[i-1][j].second | (table[i][j].first < table[i-1][j-1].first);
					}
				}
			}
			if (N>0 && K>0 && table[N][K].second == true)
			{
				std::cerr << "N=" << N << " K=" << K << " v1=" << table[N][K].first << " v2=" << table[N-1][K-1].first << " v3=" << table[N-1][K].first << std::endl;
				throw std::runtime_error("binomial_coefficient(): integer addition overflow");
			}
			return table[N][K].first;
		}

		template<typename UInt = std::size_t>
		UInt multiset_coefficient(std::size_t N, std::size_t K)
		{
			if (N == 0)
				return (K == 0) ? UInt(1) : UInt(0);
			return binomial_coefficient<UInt>(N + K - 1, K);
		}


		template<std::size_t N, std::size_t K, typename UInt = std::size_t>
		struct binomial_coefficient_detail_t
		{
			typedef binomial_coefficient_detail_t<K==0? 0 : N-1,K==0? 1 : K-1, UInt> left_t;
			typedef binomial_coefficient_detail_t<(K>N-1)? 0 : N-1, (K>N-1)? 1: K, UInt> right_t;
#ifdef _MSC_VER
#pragma warning(suppress: 4307)
			static const UInt value = left_t::value + right_t::value;
#else
			static const UInt value = left_t::value + right_t::value;
#endif
			static const bool overflow = (value < left_t::value) | (value < right_t::value) | left_t::overflow | right_t::overflow;
		};
		template<typename UInt> struct binomial_coefficient_detail_t<0,0,UInt> { static const UInt value = 1; static const bool overflow = false; };
		template<std::size_t K, typename UInt> struct binomial_coefficient_detail_t<0, K, UInt> { static const UInt value = 0; static const bool overflow = false; };

		template<std::size_t N, std::size_t K, typename UInt = std::size_t>
		struct binomial_coefficient_t
		{
			static const std::size_t N2 = (K <= N) ? N : 0;
			static const std::size_t K2 = (N - K) < K ? (N - K) : K;
			static const UInt value = binomial_coefficient_detail_t<N2, K2, UInt>::value;
			static const bool overflow = binomial_coefficient_detail_t<N2, K2, UInt>::overflow;
		};
		template<std::size_t K, typename UInt>
		struct binomial_coefficient_t<0, K, UInt>
		{
			static const UInt value = K==0 ? 1 : 0;
			static const bool overflow = false;
		};

		template<std::size_t N, std::size_t K, typename UInt = std::size_t>
		struct multiset_coefficient_t
		{
			static const UInt value = binomial_coefficient_t<N + K - 1, K, UInt>::value;
			static const bool overflow = binomial_coefficient_t<N + K - 1, K, UInt>::overflow;
		};
		template<std::size_t K, typename UInt>
		struct multiset_coefficient_t<0, K, UInt>
		{
			static const UInt value = 0;
			static const bool overflow = false;
		};
		template<typename UInt>
		struct multiset_coefficient_t<0, 0, UInt>
		{
			static const UInt value = 1;
			static const bool overflow = false;
		};

		template<std::size_t N, std::size_t MAXD, std::size_t D, typename UInt, std::size_t step, bool ignore>
		struct max_degree_fits_int_helper_t {
			static const bool next_has_overflow = multiset_coefficient_t<N+1, D+step, UInt>::overflow || (D+step>MAXD);
			static const std::size_t value = 
				max_degree_fits_int_helper_t<N,MAXD,D+step,UInt,step,next_has_overflow>::value
				+ max_degree_fits_int_helper_t<N,MAXD,D,UInt,step/2,!next_has_overflow>::value;
		};
		template<std::size_t N, std::size_t MAXD, std::size_t D, typename UInt>
		struct max_degree_fits_int_helper_t<N,MAXD,D,UInt,1,false> {
			static const bool next_has_overflow = multiset_coefficient_t<N+1, D+1, UInt>::overflow || (D+1>MAXD);
			static const std::size_t value =
				!next_has_overflow
				? max_degree_fits_int_helper_t<N,MAXD,D+1,UInt,1,next_has_overflow>::value
				: D;
		};
		template<std::size_t N, std::size_t MAXD, std::size_t D, typename Int, std::size_t step>
		struct max_degree_fits_int_helper_t<N,MAXD,D,Int,step,true> {
			static const std::size_t value = 0;
		};
		template<std::size_t N, std::size_t MAXD, typename Int>
		struct max_degree_fits_int_t {
			static const std::size_t value = max_degree_fits_int_helper_t<N,MAXD,0,Int,(1<<3),false>::value;
		};

		// # bits needed to store value N
		template<std::size_t N, std::size_t E=0> struct nrbits_t      { static const std::size_t value = nrbits_t<(N>>1),(E+1)>::value; };
		template<std::size_t E>                  struct nrbits_t<1,E> { static const std::size_t value = E+1; };
		template<std::size_t E>                  struct nrbits_t<0,E> { };

		// # bytes needed to store values 0,...,N for N>0
		template<std::size_t N>       struct nrbytes_t    { static const unsigned int value = nrbytes_t<(N>>8)>::value + 1; };
		template<>                    struct nrbytes_t<0> { static const unsigned int value = 0; };

		// determine least integer for # bytes
		template<std::size_t nrbytes> struct least_unsigned_integer_t    {  };
		template<>                    struct least_unsigned_integer_t < 0 > { typedef std::uint8_t  type; };
		template<>                    struct least_unsigned_integer_t < 1 > { typedef std::uint8_t  type; };
		template<>                    struct least_unsigned_integer_t < 2 > { typedef std::uint16_t type; };
		template<>                    struct least_unsigned_integer_t < 3 > { typedef std::uint32_t type; };
		template<>                    struct least_unsigned_integer_t < 4 > { typedef std::uint32_t type; };
		template<>                    struct least_unsigned_integer_t < 5 > { typedef std::uint64_t type; };
		template<>                    struct least_unsigned_integer_t < 6 > { typedef std::uint64_t type; };
		template<>                    struct least_unsigned_integer_t < 7 > { typedef std::uint64_t type; };
		template<>                    struct least_unsigned_integer_t < 8 > { typedef std::uint64_t type; };

		// determine least integer for # bytes that is not an unsigned char (used for iostream: prevent output as character)
		template<std::size_t nrbytes> struct least_nonchar_unsigned_integer_t       { typedef std::uint16_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 0 > { typedef std::uint16_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 1 > { typedef std::uint16_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 2 > { typedef std::uint16_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 3 > { typedef std::uint32_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 4 > { typedef std::uint32_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 5 > { typedef std::uint64_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 6 > { typedef std::uint64_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 7 > { typedef std::uint64_t type; };
		template<>                    struct least_nonchar_unsigned_integer_t < 8 > { typedef std::uint64_t type; };

		template<typename Int>
		inline unsigned bitscanreverse(Int x)
		{
			unsigned i = 0;
			while (x != 0)
			{
				x >>= 1;
				++i;
			}
			return i-1;
		}
		template<typename Int>
		inline unsigned bitscanforward(Int x)
		{
			if (x == 0)
				return ~unsigned(0);
			unsigned i = 0;
			while ((x&1) == 0)
			{
				x >>= 1;
				++i;
			}
			return i;
		}

		// assumes n > 1, i > 2, try to find factors of n starting with i, i+2, ...
		inline std::vector<std::size_t> factor_int(std::size_t n, std::size_t i)
		{
			std::vector<std::size_t> ret;

			for (; (n % i) != 0; i += 2)
				;

			// i divides n
			do
			{
				ret.push_back(i);
				n /= i;
			} while ((n % i) == 0);

			// all factors found
			if (n == 1)
				return ret;

			// recursive call to find remaining factors
			std::vector<std::size_t> tmp = factor_int(n, i + 2);
			ret.insert(ret.end(), tmp.begin(), tmp.end());
			return ret;
		}

		inline std::vector<std::size_t> factor_int(std::size_t n)
		{
			std::vector<std::size_t> ret;
			if (n == 0)
				return ret;

			while ((n & 1) == 0)
			{
				ret.push_back(2);
				n >>= 1;
			}
			if (n == 1)
				return ret;

			std::vector<std::size_t> tmp = factor_int(n, 3);
			ret.insert(ret.end(), tmp.begin(), tmp.end());
			return ret;
		}

		void hash_combine(std::size_t& x, const std::size_t in)
		{
			x ^= in + 0x9e3779b9 + (x << 6) + (x >> 2);
		}

		// helper for gb::detail::is_prime
		template<std::size_t P, std::size_t B, std::size_t E, bool validrange>
		struct has_divisor_in_range
		{
			static const std::size_t mid = (B+E)/2;
			static const bool value = 
				  has_divisor_in_range<P, B, mid, (B<=mid) & ((B*B)<=P) >::value 
				| has_divisor_in_range<P, mid+1, E, (mid+1<=E) & ((mid+1)*(mid+1)<=P) >::value;
		};
		template<std::size_t P, std::size_t B, std::size_t E>
		struct has_divisor_in_range<P,B,E,false>
		{
			static const bool value = false;
		};
		template<std::size_t P, std::size_t B>
		struct has_divisor_in_range<P,B,B,true>
		{
			static const bool value = ((P % B) == 0);
		};
		// compile-time check if a given number is prime.
		template<std::size_t P>
		struct is_prime
		{
			static const bool value = (P >= 2) & ! has_divisor_in_range<P, 2, P-1, (2<=P-1)>::value;
		};

	} // namespace detail

} // namespace gb

#endif // M4GB_DETAIL_HPP
