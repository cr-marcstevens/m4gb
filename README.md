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

Optional: install boost locally (e.g., in your home directory, default prefix: $(pwd)/boost-VERSION):

	BOOST_VERSION=1.65.1 BOOST_INSTALL_PREFIX=$HOME/boost/boost-1.65.1 ./install_boost.sh

Optional: install openf4 locally (default prefix: $(pwd)/openf4):

	./install_openf4.sh

Configuring:

	autoreconf --install
	./configure [--with-boost=$HOME/boost/boost-1.65.1] [--with-openf4=$(pwd)/openf4]

Run tests:

	make check
  
Compiling a specialized solver `bin/solver_BACKEND_MAXVARS_FIELDSIZE` with `BACKEND` equal to `m4gb`, `openf4` or `fgb` for a system of `MAXVARS` variables and finite field of size `FIELDSIZE`:

	make MAXVARS=20 FIELDSIZE=31 m4gb

	make MAXVARS=20 FIELDSIZE=31 openf4

	make MAXVARS=20 FIELDSIZE=31 fgb

