EXTRA_DIST       = README.md LICENSE.txt
ACLOCAL_AMFLAGS  = -I m4



MAXVARS ?= 20
FIELDSIZE ?= 31
EXTRA_CPPFLAGS=-DMAXVARS=${MAXVARS} -DFIELDSIZE=${FIELDSIZE}
SOLVERSUFFIX=_n${MAXVARS}_gf${FIELDSIZE}



EXTRA_PROGRAMS=bin/solver_m4gb

bin_solver_m4gb_SOURCES=src/m4gb.cpp contrib/runtime_stats.cpp
bin_solver_m4gb_CPPFLAGS=${M4GB_CPPFLAGS} ${EXTRA_CPPFLAGS}

m4gb: bin/solver_m4gb${SOLVERSUFFIX}
	@echo "Executable: bin/solver_m4gb${SOLVERSUFFIX}"

bin/solver_m4gb${SOLVERSUFFIX}:
	-rm -f $(bin_solver_m4gb_OBJECTS) bin/solver_m4gb
	$(MAKE) bin/solver_m4gb
	mv bin/solver_m4gb bin/solver_m4gb${SOLVERSUFFIX}
	-rm -f $(bin_solver_m4gb_OBJECTS)



if HAVE_OPENF4
EXTRA_PROGRAMS+=bin/solver_openf4

bin_solver_openf4_SOURCES=src/openf4.cpp contrib/runtime_stats.cpp
bin_solver_openf4_CPPFLAGS=${OPENF4_CPPFLAGS} ${EXTRA_CPPFLAGS}
bin_solver_openf4_LDFLAGS=${OPENF4_LDFLAGS}
bin_solver_openf4_LDADD=${OPENF4_LIB}

openf4: bin/solver_openf4${SOLVERSUFFIX}
	@echo "Executable: bin/solver_openf4${SOLVERSUFFIX}"

bin/solver_openf4${SOLVERSUFFIX}:
	-rm -f $(bin_solver_openf4_OBJECTS) bin/solver_openf4
	$(MAKE) bin/solver_openf4
	mv bin/solver_openf4 bin/solver_openf4${SOLVERSUFFIX}
	-rm -f $(bin_solver_openf4_OBJECTS)
endif



if HAVE_FGBLIB
EXTRA_PROGRAMS+=bin/solver_fgb

bin_solver_fgb_SOURCES=src/fgb.cpp contrib/runtime_stats.cpp
bin_solver_fgb_CPPFLAGS=${FGB_CPPFLAGS} ${EXTRA_CPPFLAGS}
bin_solver_fgb_LDFLAGS=${FGB_LDFLAGS}
bin_solver_fgb_LDADD=${FGB_LIB}

fgb: bin/solver_fgb${SOLVERSUFFIX}
	@echo "Executable: bin/solver_fgb${SOLVERSUFFIX}"

bin/solver_fgb${SOLVERSUFFIX}:
	-rm -f $(bin_solver_fgb_OBJECTS) bin/solver_fgb
	$(MAKE) bin/solver_fgb
	mv bin/solver_fgb bin/solver_fgb${SOLVERSUFFIX}
	-rm -f $(bin_solver_fgb_OBJECTS)
endif



bin/solver_m4gb_%:
	$(MAKE) $@ MAXVARS=`echo $@ | cut -d'_' -f3 | tr -d n` FIELDSIZE=`echo $@ | cut -d'_' -f4 | tr -d gf`
bin/solver_openf4_%:
	$(MAKE) $@ MAXVARS=`echo $@ | cut -d'_' -f3 | tr -d n` FIELDSIZE=`echo $@ | cut -d'_' -f4 | tr -d gf`
bin/solver_fgb_%:
	$(MAKE) $@ MAXVARS=`echo $@ | cut -d'_' -f3 | tr -d n` FIELDSIZE=`echo $@ | cut -d'_' -f4 | tr -d gf`

clean-local:
	-rm -f bin/solver_m4gb* bin/solver_openf4* bin/solver_fgb*



check_PROGRAMS = tests/test_detail tests/test_gf_p_simple tests/test_gf_2n_simple tests/test_monomial_degrevlex tests/test_polynomial tests/test_parser tests/test_runtime_stats

tests_test_detail_SOURCES               = tests/lib_detail.cpp
tests_test_gf_p_simple_SOURCES          = tests/lib_gf_p_simple.cpp
tests_test_gf_2n_simple_SOURCES         = tests/lib_gf_2n_simple.cpp
tests_test_monomial_degrevlex_SOURCES   = tests/lib_monomial_degrevlex.cpp
tests_test_polynomial_SOURCES           = tests/lib_polynomial.cpp
tests_test_parser_SOURCES               = tests/lib_parser.cpp
tests_test_runtime_stats_SOURCES        = tests/contrib_runtime_stats.cpp

tests/gf%.sh:
	cd tests && $(LN_S) solver_test.sh $(notdir $@)



TESTS = $(check_PROGRAMS)

TESTS += tests/gf31.16.m4gb.sh tests/gf256.16.m4gb.sh

if HAVE_OPENF4
TESTS += tests/gf31.16.openf4.sh tests/gf256.16.openf4.sh
endif

if HAVE_FGBLIB
TESTS += tests/gf31.16.fgb.sh tests/gf256.16.fgb.sh
endif
