EXTRA_DIST       = README.md LICENSE.txt
ACLOCAL_AMFLAGS  = -I m4



MAXVARS ?= 20
FIELDSIZE ?= 31
EXTRA_CPPFLAGS=-DMAXVARS=${MAXVARS} -DFIELDSIZE=${FIELDSIZE}
SOLVERSUFFIX=_n${MAXVARS}_gf${FIELDSIZE}



EXTRA_PROGRAMS=bin/solver_m4gb

bin_solver_m4gb_SOURCES=src/m4gb.cpp contrib/runtime_stats.cpp
bin_solver_m4gb_CPPFLAGS=${M4GB_CPPFLAGS} ${EXTRA_CPPFLAGS}

if HAVE_GIVARO
bin_solver_m4gb_CPPFLAGS+=${GIVARO_CPPFLAGS} -std=gnu++11
bin_solver_m4gb_LDFLAGS=${GIVARO_LDFLAGS}
bin_solver_m4gb_LDADD=${GIVARO_LIB} ${GMP_LIB} ${GMPXX_LIB}
endif

m4gb: bin/solver_m4gb${SOLVERSUFFIX}
	@echo "Executable: $<"

bin/solver_m4gb${SOLVERSUFFIX}:
	-rm -f $(bin_solver_m4gb_OBJECTS) bin/solver_m4gb
	$(MAKE) bin/solver_m4gb
	mv bin/solver_m4gb $@
	-rm -f $(bin_solver_m4gb_OBJECTS)

bin/solver_m4gb_%:
	$(MAKE) $@ MAXVARS=`echo $@ | cut -d'_' -f3 | tr -d n` FIELDSIZE=`echo $@ | cut -d'_' -f4 | tr -d gf`



if HAVE_OPENF4
EXTRA_PROGRAMS+=bin/solver_openf4

bin_solver_openf4_SOURCES=src/openf4.cpp contrib/runtime_stats.cpp
bin_solver_openf4_CPPFLAGS=${OPENF4_CPPFLAGS} ${EXTRA_CPPFLAGS}
bin_solver_openf4_LDFLAGS=${OPENF4_LDFLAGS}
bin_solver_openf4_LDADD=${OPENF4_LIB}

openf4: bin/solver_openf4${SOLVERSUFFIX}
	@echo "Executable: $<"

bin/solver_openf4${SOLVERSUFFIX}:
	-rm -f $(bin_solver_openf4_OBJECTS) bin/solver_openf4
	$(MAKE) bin/solver_openf4
	mv bin/solver_openf4 $@
	-rm -f $(bin_solver_openf4_OBJECTS)

bin/solver_openf4_%:
	$(MAKE) $@ MAXVARS=`echo $@ | cut -d'_' -f3 | tr -d n` FIELDSIZE=`echo $@ | cut -d'_' -f4 | tr -d gf`
endif



if HAVE_FGB
EXTRA_PROGRAMS+=bin/solver_fgb

bin_solver_fgb_SOURCES=src/fgb.cpp contrib/runtime_stats.cpp
bin_solver_fgb_CPPFLAGS=${FGB_CPPFLAGS} ${EXTRA_CPPFLAGS}
bin_solver_fgb_LDFLAGS=${FGB_LDFLAGS}
bin_solver_fgb_LDADD=${FGB_LIB}

fgb: bin/solver_fgb${SOLVERSUFFIX}
	@echo "Executable: $<"

bin/solver_fgb${SOLVERSUFFIX}:
	-rm -f $(bin_solver_fgb_OBJECTS) bin/solver_fgb
	$(MAKE) bin/solver_fgb
	mv bin/solver_fgb $@
	-rm -f $(bin_solver_fgb_OBJECTS)

bin/solver_fgb_%:
	$(MAKE) $@ MAXVARS=`echo $@ | cut -d'_' -f3 | tr -d n` FIELDSIZE=`echo $@ | cut -d'_' -f4 | tr -d gf`
endif



EXTRA_PROGRAMS+=bin/generator
bin_generator_SOURCES=src/generator.cpp
bin_generator_CPPFLAGS=${EXTRA_CPPFLAGS} -DDEG=${DEG}

generator: bin/generator_n${MAXVARS}_d${DEG}_gf${FIELDSIZE}
	@echo "Executable: $<"

bin/generator_n${MAXVARS}_d${DEG}_gf${FIELDSIZE}:
	-rm -f $(bin_generator_OBJECTS) bin/generator
	$(MAKE) bin/generator
	mv bin/generator $@
	-rm -f $(bin_generator_OBJECTS)



clean-local:
	-rm -f bin/solver_m4gb* bin/solver_openf4* bin/solver_fgb*
	-rm -f bin/generator*



check_PROGRAMS = tests/test_detail tests/test_gf_p_simple tests/test_gf_2n_simple tests/test_monomial_degrevlex tests/test_polynomial tests/test_parser tests/test_runtime_stats

tests_test_detail_SOURCES               = tests/lib_detail.cpp
tests_test_gf_p_simple_SOURCES          = tests/lib_gf_p_simple.cpp
tests_test_gf_2n_simple_SOURCES         = tests/lib_gf_2n_simple.cpp
tests_test_monomial_degrevlex_SOURCES   = tests/lib_monomial_degrevlex.cpp
tests_test_polynomial_SOURCES           = tests/lib_polynomial.cpp
tests_test_parser_SOURCES               = tests/lib_parser.cpp
tests_test_runtime_stats_SOURCES        = tests/contrib_runtime_stats.cpp


if HAVE_GIVARO
check_PROGRAMS+=tests/test_gf_pn_givaro
tests_test_gf_pn_givaro_SOURCES=tests/lib_gf_pn_givaro.cpp
tests_test_gf_pn_givaro_CPPFLAGS=${GIVARO_CPPFLAGS}
tests_test_gf_pn_givaro_LDFLAGS=${GIVARO_LDFLAGS}
tests_test_gf_pn_givaro_LDADD=${GIVARO_LIB} ${GMP_LIB} ${GMPXX_LIB}

check_PROGRAMS+=tests/test_gf_p_givaro
tests_test_gf_p_givaro_SOURCES=tests/lib_gf_p_givaro.cpp
tests_test_gf_p_givaro_CPPFLAGS=${GIVARO_CPPFLAGS} -std=gnu++11
tests_test_gf_p_givaro_LDFLAGS=${GIVARO_LDFLAGS}
tests_test_gf_p_givaro_LDADD=${GIVARO_LIB} ${GMP_LIB} ${GMPXX_LIB}
endif


tests/gf%.sh:
	cd tests && $(LN_S) solver_test.sh $(notdir $@)



TESTS = $(check_PROGRAMS)

TESTS += tests/gf31.16.m4gb.sh tests/gf256.16.m4gb.sh tests/gf4294967291.16.m4gb.sh

if HAVE_OPENF4
TESTS += tests/gf31.16.openf4.sh tests/gf256.16.openf4.sh
endif

if HAVE_FGB
TESTS += tests/gf31.16.fgb.sh
endif
