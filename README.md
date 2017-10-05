# M4GB - Efficient Groebner Basis algorithm #

## Project is still being cleaned-up and commited here in phases. ##

## Requirements ##

- autotools
- C++11 compatible compiler
- Optional: OpenF4 (can be used as another backend instead of the M4GB algorithm, facilitating benchmarking)

## Building ##

Optional: install openf4 locally

	./install_openf4.sh

Configuring

	autoreconf --install
	./configure
  
Compiling

	make

Run tests

	make check
  
