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

namespace gb
{

	namespace detail
	{

		template<typename Int = std::size_t>
		Int binomial_coefficient(std::size_t N, std::size_t K)
		{
			static std::vector< std::vector<Int> > table(1, std::vector<Int>(1, Int(1)));
			if ((N < 0) || (K < 0) || (K > N))
				return Int(0);
			if (N >= table.size())
			{
				table.reserve(N+1);
				for (std::size_t i = table.size(); i <= N; ++i)
				{
					table.emplace_back(i+1, Int(0));
					table[i][0] = table[i][i] = Int(1);
					for (std::size_t j = 1; j < i; ++j)
						table[i][j] = table[i-1][j-1] + table[i-1][j];
				}
			}
			if (N>0 && K>0 && table[N][K] < table[N-1][K-1])
				throw std::runtime_error("binomial_coefficient(): integer addition overflow");
			return table[N][K];
		}

		template<typename Int = std::size_t>
		Int multiset_coefficient(std::size_t N, std::size_t K)
		{
			if (N == 0)
				return (K == 0) ? Int(1) : Int(0);
			return binomial_coefficient<Int>(N + K - 1, K);
		}


		template<std::size_t N, std::size_t K>
		struct binomial_coefficient_detail_t
		{
			static const std::size_t value = 
				binomial_coefficient_detail_t<K==0? 0 : N-1,K==0? 1 : K-1>::value 
				+ binomial_coefficient_detail_t<(K>N-1)? 0 : N-1, (K>N-1)? 1: K>::value;
		};
		template<> struct binomial_coefficient_detail_t<0,0> { static const std::size_t value = 1; };
		template<std::size_t K> struct binomial_coefficient_detail_t<0, K> { static const std::size_t value = 0; };

		template<std::size_t N, std::size_t K>
		struct binomial_coefficient_t
		{
			static const std::size_t N2 = (K <= N) ? N : 0;
			static const std::size_t K2 = (N - K) < K ? (N - K) : K;
			static const std::size_t value = binomial_coefficient_detail_t<N2, K2>::value;
		};
		template<std::size_t K>
		struct binomial_coefficient_t<0, K>
		{
			static const std::size_t value = K==0 ? 1 : 0;
		};

		template<std::size_t N, std::size_t K>
		struct multiset_coefficient_t
		{
			static const std::size_t value = binomial_coefficient_t<N + K - 1, K>::value;
		};
		template<std::size_t K>
		struct multiset_coefficient_t<0, K>
		{
			static const std::size_t value = 0;
		};
		template<>
		struct multiset_coefficient_t<0, 0>
		{
			static const std::size_t value = 1;
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

	} // namespace detail

} // namespace gb

#endif // M4GB_DETAIL_HPP
