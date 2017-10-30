#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include "config.hpp"

typedef gb::getfield<FIELDSIZE>::type field_t;
typedef field_t::gfelm_t coefficient_t;
typedef gb::monomial_degrevlex_traits_uint64<MAXVARS, DEG> monomial_int_traits_t;

typedef monomial_int_traits_t::int_monomial_t monomial_int_t;
typedef monomial_int_traits_t::static_monomial_t monomial_static_t;

typedef gb::polynomial_simple_t<monomial_int_traits_t, field_t> polynomial_t;

const auto NMONOMIALS = gb::detail::multiset_coefficient_t<MAXVARS + 1, DEG>::value;

int main()
{
    std::array<monomial_static_t, NMONOMIALS> monomials;
    std::array<std::array<coefficient_t, NMONOMIALS>, NPOLYS> coeff_matrix;
    std::array<coefficient_t, MAXVARS> solution;

    /* generate all monomials */
    for(std::size_t i = 0; i < NMONOMIALS; ++i)
        monomials[i] = monomial_int_t(i);

    /* random number generator */
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, FIELDSIZE - 1);

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
    const std::string outputname = std::to_string(FIELDSIZE) + "_" + \
        "n" + std::to_string(MAXVARS) + "_" + \
        "m" + std::to_string(NPOLYS);
    std::ofstream in_os(outputname + ".in");
    std::ofstream ans_os(outputname + ".ans");

    /* write input file */
    in_os << "$fieldsize " << std::to_string(FIELDSIZE) << std::endl;
    in_os << "$vars " << std::to_string(MAXVARS) << " X" << std::endl;
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
    std::vector<polynomial_t> gb(NPOLYS);

    std::reverse(solution.begin(), solution.end());
    for(std::size_t i = 1; i <= MAXVARS; ++i)
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
    std::cout << "Input file               : " << outputname + ".in" << std::endl;
    std::cout << "Answer file              : " << outputname + ".ans" << std::endl;

    return 0;
}
