#include "../lib/detail.hpp"

#include <iostream>
#include <stdexcept>

int test()
{
	for (size_t i = 0; i < 1000; ++i)
	{
		if (gb::binomial_coefficient(i,0) != 1) return 1;
		if (gb::binomial_coefficient(i,1) != i) return 1;
		if (gb::binomial_coefficient(i,i) != 1) return 1;
		if (gb::binomial_coefficient(i,i+1) != 0) return 1;
	}
	if (gb::binomial_coefficient(11,5) != 462) return 1;
	if (gb::binomial_coefficient(37,25) != 1852482996) return 1;
	return 0;
}

int main(int argc, char** argv)
{
	return test();
}
