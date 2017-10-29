#!/bin/bash

CPPFILENAME="generator.cpp"
DEG=2 #default degree is quadratic

cleanup()
{
    if [ -f $CPPFILENAME ]; then
        rm -f $CPPFILENAME
    fi
    if [ -f "a.out" ]; then
        rm -f a.out
    fi
}

display_help()
{
    echo "Random overdefined polyomial equations' generator over a finite field"
    echo "Usage : $0 -f <fieldsize> -n <no. of variables> -m <no. of equations> [-d <degree>]"
    echo "Arguments"
    echo "  -h                    : display this help"
    echo "  -f <fieldsize>        : set the size of the finite field"
    echo "  -n <no. of variables> : set the number of variables"
    echo "  -m <no. of equations> : set the number of equations"
    echo "  -d <degree>           : (optional) set the degree (default=2)"
    exit 0
}

cleanup
while getopts :hf:n:m:d: option
do
    case "$option" in
        h)
            display_help
            ;;
        n)
            N=$OPTARG
            ;;
        m)
            M=$OPTARG
            ;;
        f)
            FIELDSIZE=$OPTARG
            ;;
        d)
            DEG=$OPTARG
            ;;
        \?)
            echo "Invalid options: -$OPTARG" >&2
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument" >&2
            exit 1;
            ;;
    esac
done

if [ -z "$FIELDSIZE" ]; then
    echo "The field size is not set (use -f)"
    exit 1
fi
if [ -z "$M" ]; then
    echo "The number of equations is not set (use -m)"
    exit 1
fi
if [ -z "$N" ]; then
    echo "The number of variables is not set (use -n)"
    exit 1
fi

re='^[0-9]+$'
if ! [[ $FIELDSIZE =~ $re ]] || [ $FIELDSIZE -lt 2 ]; then
    echo "The field size (-f) must be a positive integer >= 2"
    exit 1
fi
if ! [[ $N =~ $re ]] || [ $N -lt 1 ]; then
    echo "The number of variables (-n) must be a positive integer"
    exit 1
fi
if ! [[ $M =~ $re ]] || [ $M -lt 1 ]; then
    echo "The number of equations (-m) must be a positive integer"
    exit 1
fi
if ! [[ $DEG =~ $re ]] || [ $DEG -lt 1 ]; then
    echo "The degree (-d) must be an integer"
    exit 1
fi

if [ $M -le $N ]; then
    echo "the number of equations must be greater than the number of variables"
    exit 1
fi

echo "size of the finite field : $FIELDSIZE"
echo "number of variables      : $N"
echo "number of equations      : $M"
echo "degree                   : $DEG"

echo "\
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include \"../lib/monomial_base.hpp\"
#include \"../lib/monomial_degrevlex.hpp\"
#include \"../solver/config.hpp\" //for gb::getfield

typedef gb::getfield<$FIELDSIZE>::type field_t;
typedef field_t::gfelm_t coefficient_t;
typedef gb::monomial_degrevlex_traits_uint64<$N, $DEG> monomial_int_traits_t;

typedef monomial_int_traits_t::int_monomial_t monomial_int_t;
typedef monomial_int_traits_t::static_monomial_t monomial_static_t;

typedef gb::polynomial_simple_t<monomial_int_traits_t, field_t> polynomial_t;

const auto NMONOMIALS = gb::detail::multiset_coefficient_t<$N + 1, $DEG>::value;

int main()
{
    std::array<monomial_static_t, NMONOMIALS> monomials;
    std::array<std::array<coefficient_t, NMONOMIALS>, $M> coeff_matrix;
    std::array<coefficient_t, $N> solution;

    /* generate all monomials */
    for(std::size_t i = 0; i < NMONOMIALS; ++i)
        monomials[i] = monomial_int_t(i);

    /* random number generator */
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, $FIELDSIZE - 1);

    /* generate random coefficients */
    for (auto & row : coeff_matrix)
        for (auto & c : row)
            c = dist(gen);

    /* generate solution */
    for(auto & s : solution)
        s = dist(gen);

    /* substitute polynomials */
    for(auto & row : coeff_matrix)
    {
        coefficient_t sum_val = 0;
        for(std::size_t idx = 0; idx < NMONOMIALS; ++idx)
        {
            if (row[idx] == 0)
                continue;

            coefficient_t term_val = row[idx];
            const monomial_static_t & m = monomials[idx];

            for(const auto & ie : m)
            {
                unsigned i = ie.first;
                unsigned e = ie.second;
                do
                {
                    term_val *= solution[i];
                    --e;
                } while(e);
            }
            sum_val += term_val;
        }

        /* adjust constant term */
        row[0] -= sum_val;
    }

    /* print file */
    const std::string outputname = \"${FIELDSIZE}_n${N}_m${M}\";
    std::ofstream in_os(outputname + \".in\");
    std::ofstream ans_os(outputname + \".ans\");

    /* write input file */
    in_os << \"\$fieldsize $FIELDSIZE\" << std::endl;
    in_os << \"\$vars $N X\" << std::endl;
    for(const auto & row : coeff_matrix)
    {
        std::vector<std::pair<coefficient_t, monomial_static_t>> v;
        for(std::size_t i = 0; i < NMONOMIALS; ++i)
        {
            if (row[i] == 0)
               continue;
            v.emplace_back(std::make_pair(row[i], monomials[i]));
        }
        polynomial_t f(v.begin(), v.end());
        in_os << f << std::endl;
    }

    /* write answer file */
    std::vector<polynomial_t> gb($M);

    std::reverse(solution.begin(), solution.end());
    for(std::size_t i = 1; i <= $N; ++i)
    {
        std::vector<std::pair<coefficient_t, monomial_static_t>> v;
        v.emplace_back(std::make_pair(1, monomials[i]));
        v.emplace_back(std::make_pair(-solution[i-1], monomials[0]));
        polynomial_t f(v.begin(), v.end());
        ans_os << f << std::endl;
    }

    in_os.close();
    ans_os.close();

    std::cout << std::endl;
    std::cout << \"Input file               : \" << outputname + \".in\" << std::endl;
    std::cout << \"Answer file              : \" << outputname + \".ans\" << std::endl;

    return 0;
}" > $CPPFILENAME

g++ -std=c++11 -march=native -O3 -Wall $CPPFILENAME
./a.out
cleanup
