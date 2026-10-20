# Default optimization level
O ?= 2

TESTS = $(patsubst %.cc,%,$(sort $(wildcard *[0-9][0-9][0-9].cc t/*.cc)))
SPECIALTESTS = $(patsubst %.cc,%,$(sort $(wildcard tt/*.cc)))
TORTURETESTS = $(patsubst %.cc,%,$(sort $(wildcard torture/*.cc)))
ALLTESTS = $(TESTS) $(SPECIALTESTS) $(TORTURETESTS)

all: sh61 $(TESTS) $(TORTURETESTS)

-include build/rules.mk

special: $(SPECIALTESTS)

%.o: %.cc sh61.hh $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) $(O) -o $@ -c,COMPILE,$<)

sh61: sh61.o helpers.o
	$(call run,$(CXX) $(CXXFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),LINK $@)

sleep61: sleep61.cc
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),BUILD $@)

# Tests are built without sanitizers, which slow down `fork`
TESTCXXFLAGS = $(filter-out -fsanitize% -fno-sanitize%,$(CXXFLAGS))

shtest.o: shtest.cc shtest.hh $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(TESTCXXFLAGS) $(DEPCFLAGS) $(O) -o $@ -c,COMPILE,$<)

$(patsubst %,%.o,$(ALLTESTS)): %.o: %.cc shtest.hh $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(TESTCXXFLAGS) $(DEPCFLAGS) $(O) -o $@ -c,COMPILE,$<)

$(ALLTESTS): %: %.o shtest.o
	$(call run,$(CXX) $(TESTCXXFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),LINK $@)

ifneq ($(filter -fsanitize=leak,$(CXXFLAGS)),)
LEAKCHECK = --leak
endif

ifneq ($(LEAKCHECK),)
CHECKENV = ASAN_OPTIONS=detect_leaks=1
endif

# `$(call colorecho,COLOR,TEXT)` prints TEXT, in bold color COLOR if
# standard output is a terminal. `$(call runtests,TESTS)` runs each test
# in TESTS, printing a green line if it passes and a red line if it fails.
colorecho = if [ -t 1 -a -t 2 ]; then printf '\033[1;$(1)m%s\033[m\n' "$(2)"; else echo "$(2)"; fi
runtests = @if [ -z "$(1)" ]; then echo "*** No matching tests" 1>&2; exit 1; fi; \
	for t in $(1); do $(CHECKENV) ./$$t; s=$$?; if [ $$s = 0 ]; then $(call colorecho,32,$$t passed); else $(call colorecho,31,$$t FAILED with exit status $$s); exit 1; fi; done

check: check-default check-torture
	@$(if $(KEEP_GOING),,$(call colorecho,32,All tests passed))
check-default: $(patsubst %,check-%,$(TESTS))
check-special: $(patsubst %,check-%,$(SPECIALTESTS))
check-torture: $(patsubst %,check-%,$(TORTURETESTS))

# `make check-NAME` runs the tests in t/, tt/, and torture/ whose names start
# with NAME
PERCENT := %
.SECONDEXPANSION:
check-t/%: sh61 $$(filter t/$$*$$(PERCENT),$$(TESTS))
	$(call runtests,$(filter-out sh61,$^))
check-tt/%: sh61 $$(filter tt/$$*$$(PERCENT),$$(SPECIALTESTS))
	$(call runtests,$(filter-out sh61,$^))
check-torture/%: sh61 $$(filter torture/$$*$$(PERCENT),$$(TORTURETESTS))
	$(call runtests,$(filter-out sh61,$^))
check-%: sh61 $$(filter $$*$$(PERCENT) t/$$*$$(PERCENT) tt/$$*$$(PERCENT) torture/$$*$$(PERCENT),$$(ALLTESTS))
	$(call runtests,$(filter-out sh61,$^))

clean: clean-main
clean-main:
	$(call run,rm -f sh61 $(ALLTESTS) *.o t/*.o tt/*.o torture/*.o *~ *.bak core *.core,CLEAN)
	$(call run,rm -rf out *.dSYM $(DEPSDIR))

.PRECIOUS: %.o
.PHONY: all special clean clean-main distclean \
	check check-default check-special check-torture \
	check-% check-t/% check-tt/% check-torture/%
