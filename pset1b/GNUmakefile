TESTS = $(patsubst %.cc,%,$(sort $(wildcard *[0-9][0-9][0-9].cc t/*.cc)))
SPECIALTESTS = $(patsubst %.cc,%,$(sort $(wildcard tt/*.cc)))

all: $(TESTS)

-include build/rules.mk

special: $(SPECIALTESTS)

%.o: %.cc $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) -o $@ -c,COMPILE,$<)

m61.o: m61-$(M61).cc $(M61STAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) -o $@ -c,COMPILE,$<)

$(TESTS): %: %.o m61.o $(OBJS)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)
$(SPECIALTESTS): %: %.o m61.o $(OBJS)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)

CHECKENV = ASAN_OPTIONS=detect_leaks=0

check-%: %
	$(CHECKENV) ./$^
check-t/%: t/%
	$(CHECKENV) ./$^
check-tt/%: tt/%
	$(CHECKENV) ./$^

check: check-all
check-all: $(patsubst %,check-%,$(TESTS))
check-special: $(patsubst %,check-%,$(SPECIALTESTS))

clean: clean-main
clean-main:
	$(call run,rm -f $(TESTS) $(SPECIALTESTS) *.o core *.core,CLEAN)
	$(call run,rm -rf out *.dSYM $(DEPSDIR))

distclean: clean

.PRECIOUS: %.o
.PHONY: all always clean clean-main clean-hook distclean \
	check-% check-all check-special check
