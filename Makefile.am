EXTRA_DIST       = README.md
ACLOCAL_AMFLAGS  = -I m4

TESTS = tests/test_detail tests/test_gf_p_simple tests/test_gf_2n_simple tests/test_monomial_degrevlex tests/test_polynomial tests/test_parser tests/test_runtime_stats

tests_test_detail_SOURCES               = tests/lib_detail.cpp
tests_test_gf_p_simple_SOURCES          = tests/lib_gf_p_simple.cpp
tests_test_gf_2n_simple_SOURCES         = tests/lib_gf_2n_simple.cpp
tests_test_monomial_degrevlex_SOURCES   = tests/lib_monomial_degrevlex.cpp
tests_test_polynomial_SOURCES           = tests/lib_polynomial.cpp
tests_test_parser_SOURCES               = tests/lib_parser.cpp
tests_test_runtime_stats_SOURCES        = tests/contrib_runtime_stats.cpp

check_PROGRAMS = $(TESTS)
