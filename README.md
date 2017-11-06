[![Build Status](https://travis-ci.org/cr-marcstevens/m4gb.svg?branch=master)](https://travis-ci.org/cr-marcstevens/m4gb)

# M4GB - Efficient Groebner Basis algorithm #

Rusydi Makarim, Marc Stevens, *M4GB: An efficient Groebner-basis algorithm*, ISSAC 2017

https://marc-stevens.nl/research/papers/ISSAC17-MS-M4GB.pdf

## Paper Abstract ##

We introduce a new efficient algorithm for computing Groebner-bases named M4GB.
Like Faugere's algorithm F4 it is an extension of Buchberger's algorithm that describes:
how to store already computed (tail-)reduced multiples of basis polynomials to prevent redundant work in the reduction step; 
and how to exploit efficient linear algebra for the reduction step.
In comparison to F4 it removes further redundant work in the processing of reducible monomials.
Furthermore, instead of translating the reduction of many critical pairs into the row reduction of some large matrix,
our algorithm is described more natively and is efficient while processing critical pairs one by one.
This feature implies that typically M4GB has to process fewer critical pairs than F4,
and reduces the time and data complexity 'staircase' related to the increasing degree of regularity for a sequence of problems one observes for F4.

M4GB has been designed specifically around the invariant to operate only on tail-reduced polynomials,
i.e., polynomials of which all terms except the leading term are non-reducible.
This allows it to perform full-reduction directly in the computation of a term polynomial multiplication,
where all computations are done over coefficient vectors over the non-reducible monomials.

We have implemented a version of our new algorithm tailored for dense overdefined polynomial systems as a proof of concept and made our source code publicly available here.
We have made a comparison of our implementation against the implementations of FGBlib, Magma and OpenF4 on various dense Fukuoka MQ challenge problems that we were able to compute in reasonable time and memory.
We observed that M4GB uses the least total CPU time *and* the least memory of all these implementations for those MQ problems, often by a significant factor.

### Benchmarking M4GB, Magma, FGBlib & OpenF4 ###

Benchmarks of M4GB, Magma, FGBLib & OpenF4 over random dense quadratic polynomial systems over GF(31) with *#equations = 2 * #variables*:

<img src="https://raw.github.com/cr-marcstevens/m4gb/master/testdata/graphs/type3-cputime.svg?sanitize=true" width="30%"><img src="https://raw.github.com/cr-marcstevens/m4gb/master/testdata/graphs/type3-memory.svg?sanitize=true" width="30%"><img src="https://raw.github.com/cr-marcstevens/m4gb/master/testdata/graphs/type3-cpu_x_memory.svg?sanitize=true" width="30%">

Benchmarks of M4GB, Magma, FGBLib & OpenF4 over random dense quadratic polynomial systems over GF(31) with *#equations = #variables + 1*:

<img src="https://raw.github.com/cr-marcstevens/m4gb/master/testdata/graphs/type6-cputime.svg?sanitize=true" width="30%"><img src="https://raw.github.com/cr-marcstevens/m4gb/master/testdata/graphs/type6-memory.svg?sanitize=true" width="30%"><img src="https://raw.github.com/cr-marcstevens/m4gb/master/testdata/graphs/type6-cpu_x_memory.svg?sanitize=true" width="30%">

See our paper for more benchmark information.

## Requirements ##

- autotools
- C++11 compatible compiler
- Boost C++ libraries (program_options, thread, system)
- Optional: OpenF4 (can be used as another backend instead of the M4GB algorithm, facilitating benchmarking)
- Optional: FGbLib (can also be used as another backend, only works with prime fields)

## Building ##

Optional: install boost locally (e.g., in your home directory, default prefix: $(pwd)/boost-VERSION):

	BOOST_VERSION=1.65.1 BOOST_INSTALL_PREFIX=$HOME/boost/boost-1.65.1 ./install_boost.sh

Optional: install openf4 locally (default prefix: $(pwd)/openf4):

	./install_openf4.sh

Optional: install fgb locally:

	./install_fgb.sh

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
