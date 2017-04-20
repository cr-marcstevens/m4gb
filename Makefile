include Makefile.local

all:

V=0
CXXMSG_0 = @echo "  CXX    $@"; 
CXXMSG_1 = 
CXXMSG=$(CXXMSG_$(V))
LDMSG_0 = @echo "  CXXLD  $@";
LDMSG_1 = 
LDMSG=$(LDMSG_$(V))
VMSG_0 = @
VMSG_1 =
VMSG=$(VMSG_$(V))

%.o: %.cpp $(DEP)
	$(CXXMSG) $(CXX) $(CXXFLAGS) -c -o $(basename $<).o $<

DEP=$(wildcard lib/*.hpp lib/*.cpp solver/*.hpp solver/*.cpp tests/*.hpp)
TESTS=$(subst tests/,tests/test_,$(basename $(wildcard tests/*.cpp)))
RUNTESTS=$(addsuffix .TEST,$(TESTS))
LIBS=-lrt
DISTCLEAN=openf4

.PHONY: all clean distclean check

clean:
	find -name "*.o" -delete
	find tests -name "*.log" -delete
	rm -f $(TESTS)

distclean: clean
	rm -rf $(DISTCLEAN)

### Tests ###

check: $(TESTS) $(RUNTESTS)

tests/test_%: tests/%.o $(DEP)
	$(LDMSG) $(CXX) $(LDFLAGS) -o $@ $< $(LIBS)

%.TEST: %
	@if (./$< > $<.log 2>&1 ); then echo "PASS: $<"; else echo "FAIL: $<"; fi

### OpenF4 ###

.PHONY: install-openf4
install-openf4:
	$(VMSG) if [ -d openf4 ]; then cd openf4 && git pull; else git clone https://github.com/nauotit/openf4; fi
	$(VMSG) if [ \( ! -f openf4/.ready \) -o \( `find openf4 -cnewer openf4/.ready | grep -v ".git" | wc -l` -ne 0 \) ]; then \
		rm -f openf4/.ready && \
		cd openf4 && \
		autoreconf --install && \
		./configure && \
		make clean all && \
		touch .ready ;\
	fi
	$(VMSG) echo "OpenF4 is ready!"
