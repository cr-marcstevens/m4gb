[![Build Status](https://travis-ci.org/cr-marcstevens/m4gb.svg?branch=master)](https://travis-ci.org/cr-marcstevens/m4gb)

# M4GB - Efficient Groebner Basis algorithm #

## Project is still being cleaned-up and commited here in phases. ##

## Requirements ##

- autotools
- C++11 compatible compiler
- Boost C++ libraries (program_options, thread, system)
- Optional: OpenF4 (can be used as another backend instead of the M4GB algorithm, facilitating benchmarking)
- Optional: FGbLib (can also be used as another backend)

## Building ##

Optional: install boost locally

	./install_boost.sh

Optional: install openf4 locally

	./install_openf4.sh

Configuring

	autoreconf --install
	./configure
  
Compiling

	make

Run tests

	make check
  
