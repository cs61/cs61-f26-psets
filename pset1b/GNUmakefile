TESTS = $(patsubst %.cc,%,$(sort $(wildcard *[0-9][0-9][0-9].cc t/*.cc)))
SPECIALTESTS = $(patsubst %.cc,%,$(sort $(wildcard tt/*.cc)))
TORTURETESTS = $(patsubst %.cc,%,$(sort $(wildcard torture/*.cc)))
OBJS = subtest.o

all: $(TESTS) $(TORTURETESTS)

-include build/rules.mk

special: $(SPECIALTESTS)

%.o: %.cc $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) -o $@ -c,COMPILE,$<)

m61.o: m61-$(M61).cc $(BUILDSTAMP) $(M61STAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) -o $@ -c,COMPILE,$<,$(FAILSTOP))

$(TESTS): %: %.o m61.o $(OBJS)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)
$(SPECIALTESTS): %: %.o m61.o $(OBJS)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)
$(TORTURETESTS): %: %.o m61.o $(OBJS)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)

CHECKENV = ASAN_OPTIONS=detect_leaks=0

# `$(call colorecho,COLOR,TEXT)` prints TEXT, in bold color COLOR if
# standard output is a terminal. `$(call runtests,TESTS)` runs each test
# in TESTS, printing a green line if it passes and a red line if it fails.
colorecho = if [ -t 1 -a -t 2 ]; then printf '\033[1;$(1)m%s\033[m\n' "$(2)"; else echo "$(2)"; fi
runtests = @for t in $(1); do $(CHECKENV) ./$$t; s=$$?; if [ $$s = 0 ]; then $(call colorecho,32,$$t passed); else $(call colorecho,31,$$t FAILED with exit status $$s); exit 1; fi; done

.SECONDEXPANSION:
check-%: $$(basename $$(wildcard $$(*)*.cc))
	$(call runtests,$^)
check-t/%: $$(basename $$(wildcard t/$$(*)*.cc))
	$(call runtests,$^)
check-tt/%: $$(basename $$(wildcard tt/$$(*)*.cc))
	$(call runtests,$^)
check-torture/%: $$(basename $$(wildcard torture/$$(*)*.cc))
	$(call runtests,$^)

check: check-default check-torture
	@$(if $(KEEP_GOING),,$(call colorecho,32,All tests passed))
check-default: $(patsubst %,check-%,$(TESTS))
check-special: $(patsubst %,check-%,$(SPECIALTESTS))
check-torture: $(patsubst %,check-%,$(TORTURETESTS))

clean: clean-main
clean-main:
	$(call run,rm -f $(TESTS) $(SPECIALTESTS) $(TORTURETESTS) *.o t/*.o tt/*.o torture/*.o core *.core,CLEAN)
	$(call run,rm -rf out *.dSYM $(DEPSDIR))

distclean: clean

.PRECIOUS: %.o
.PHONY: all always clean clean-main clean-hook distclean \
	check-% check-t/% check-tt/% check-torture/% \
	check check-default check-special check-torture
