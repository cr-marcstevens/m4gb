[![Build Status](https://travis-ci.org/cr-marcstevens/m4gb.svg?branch=master)](https://travis-ci.org/cr-marcstevens/m4gb)

# M4GB - Efficient Groebner Basis algorithm #

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
  
Compiling a specialized groebner-basis solver `bin/solver_BACKEND_MAXVARS_FIELDSIZE` with `BACKEND` equal to `m4gb`, `openf4` or `fgb` for a polynomial system of `MAXVARS` variables over a finite field of size `FIELDSIZE`:

	make MAXVARS=20 FIELDSIZE=31 m4gb

	make MAXVARS=20 FIELDSIZE=31 openf4

	make MAXVARS=20 FIELDSIZE=31 fgb

## Generating a random dense polynomial system ##

To generate a system of random polynomials over a (small) finitefield (prime, or 2^e):

	./generator.sh -f <fieldsize> -n <#vars> -m <#eqs> [-d <#maxdeg=2>] [-s <seed>] [-r] -o <filebasename>

To force a random root for a system use `-r`, this will rewrite the contant terms of the polynomials.
It will output the chosen root into a `.ans` file.
	
## Finding Groebner basis or roots of polynomial systems ##

To find the Groebner basis of a polynomial system, or in particular a root for an **overdefined** system if any:

	./solve.sh [-f <fieldsize>] [-n <#vars>] [-s m4gb|openf4|fgb] <inputfile>

If `fieldsize` or `#vars` are not given then it will try to detect these from the inputfile.
Then `solve.sh` will compile the corresponding solver and run it.
The default inputformat is the following:

	# Comments: everything after a '#' symbol is ignored
	$fieldsize 31          # Optional: specify finite field: GF(31)
	$vars 5 X              # Optional: type 1: specify variable name sequence: X0, X1, X2, X3, X4
	$vars X0 X1 X2 Y0 Y1   # Optional: type 2: specify variable names explicitly
	<polynomial1 in GF[varlist]>
	<polynomial2 in GF[varlist]>
	...
	
If `fieldsize` or number of variables are not declared in inputfile then these need to be specified to `solver.sh` via `-f <fieldsize>` and `-n <#vars>`.
