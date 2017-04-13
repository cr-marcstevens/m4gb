include Makefile.local

all: check

V=0
CXXMSG_0 = @echo "  CXX    $@"; 
CXXMSG_1 = 
CXXMSG=$(CXXMSG_$(V))
LDMSG_0 = @echo "  CXXLD  $@";
LDMSG_1 = 
LDMSG=$(LDMSG_$(V))

%.o: %.cpp $(DEP)
	$(CXXMSG) $(CXX) $(CXXFLAGS) -c -o $(basename $<).o $<

DEP=$(wildcard lib/*.hpp lib/*.cpp solver/*.hpp solver/*.cpp)
TESTS=$(subst tests/,tests/test_,$(basename $(wildcard tests/*.cpp)))
RUNTESTS=$(addsuffix .TEST,$(TESTS))

.PHONY: all clean check

clean:
	find -name "*.o" -delete
	rm -f $(TESTS)

check: $(TESTS) $(RUNTESTS)

tests/test_%: tests/%.o $(DEP)
	$(LDMSG) $(CXX) $(LDFLAGS) -o $@ $<

%.TEST: %
	@if ./$<; then echo "PASS: $<"; else echo "FAIL: $<"; fi

