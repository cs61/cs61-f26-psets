PROGRAMS := $(patsubst %.cc,%,$(filter-out singleslot-% slow-% stdio-% syscall-% io61.cc,$(wildcard *61.cc)))
# Tests run programs from `bin`. `bin/NAME61` links to `NAME61`, and
# `bin/LIBRARY/NAME61` is `NAME61` built with a different IO61 library, for
# `IO61=LIBRARY`.
BINLINKS = $(patsubst %,bin/%,$(PROGRAMS) $(HELPERS))
STDIOPROGRAMS = $(patsubst %,bin/stdio/%,$(PROGRAMS))
SLOWPROGRAMS = $(patsubst %,bin/slow/%,$(PROGRAMS))
SYSCALLPROGRAMS = $(patsubst %,bin/syscall/%,$(PROGRAMS))

TESTS = $(patsubst %.cc,%,$(sort $(wildcard t/*.cc)))
SPECIALTESTS = $(patsubst %.cc,%,$(sort $(wildcard tt/*.cc)))
TORTURETESTS = $(patsubst %.cc,%,$(sort $(wildcard torture/*.cc)))
ALLTESTS = $(TESTS) $(SPECIALTESTS) $(TORTURETESTS)

all: programs stdio $(TESTS) $(TORTURETESTS)

# Default optimization level
O ?= 2
-include build/rules.mk

special: $(SPECIALTESTS)

%.o: %.cc $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) $(O) -o $@ -c,COMPILE,$<)

$(PROGRAMS): %: io61.o helpers.o %.o
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)

slow-io61.o: slow-io61.cc
$(SLOWPROGRAMS): bin/slow/%: slow-io61.o helpers.o %.o
	@mkdir -p $(@D)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)

stdio-io61.o: stdio-io61.cc
$(STDIOPROGRAMS): bin/stdio/%: stdio-io61.o helpers.o %.o
	@mkdir -p $(@D)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)

syscall-io61.o: syscall-io61.cc
$(SYSCALLPROGRAMS): bin/syscall/%: syscall-io61.o helpers.o %.o
	@mkdir -p $(@D)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)


$(BINLINKS): bin/%: %
	@mkdir -p $(@D)
	@ln -sf ../$* $@

# Tests are built without sanitizers
TESTCXXFLAGS = $(filter-out -fsanitize% -fno-sanitize%,$(CXXFLAGS))

iotest.o: iotest.cc iotest.hh $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(TESTCXXFLAGS) $(DEPCFLAGS) $(O) -o $@ -c,COMPILE,$<)

$(patsubst %,%.o,$(ALLTESTS)): %.o: %.cc iotest.hh $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(TESTCXXFLAGS) $(DEPCFLAGS) $(O) -o $@ -c,COMPILE,$<)

$(ALLTESTS): %: %.o iotest.o
	$(call run,$(CXX) $(TESTCXXFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),LINK $@)

programs: $(PROGRAMS) $(HELPERS) $(BINLINKS)
stdio: $(STDIOPROGRAMS)
slow: $(SLOWPROGRAMS)
syscall: $(SYSCALLPROGRAMS)

# `$(call colorecho,COLOR,TEXT)` prints TEXT, in bold color COLOR if
# standard output is a terminal. `$(call runtests,TESTS[,MESSAGE])` runs each
# test in TESTS, one at a time, printing a green line if it passes and a red
# line if it fails, then prints MESSAGE if all tests passed. It stops at the
# first failure unless `make -k` is given. (`make -j` builds tests in parallel,
# but they always run one at a time, since many tests measure performance.)
colorecho = if [ -t 1 -a -t 2 ]; then printf '\033[1;$(1)m%s\033[m\n' "$(2)"; else echo "$(2)"; fi
runtests = @if [ -z "$(1)" ]; then echo "*** No matching tests" 1>&2; exit 1; fi; \
	f=0; for t in $(1); do ./$$t; s=$$?; \
	if [ $$s = 0 ]; then $(call colorecho,32,$$t passed); \
	else $(call colorecho,31,$$t FAILED with exit status $$s); f=1; $(if $(KEEP_GOING),,exit 1;) fi; done; \
	$(if $(2),if [ $$f = 0 ]; then $(call colorecho,32,$(2)); fi;) exit $$f

CHECKDEPS = programs stdio

check: $(CHECKDEPS) $(TESTS) $(TORTURETESTS)
	$(call runtests,$(TESTS) $(TORTURETESTS),All tests passed)
check-default: $(CHECKDEPS) $(TESTS)
	$(call runtests,$(TESTS))
check-special: $(CHECKDEPS) $(SPECIALTESTS)
	$(call runtests,$(SPECIALTESTS))
check-torture: $(CHECKDEPS) $(TORTURETESTS)
	$(call runtests,$(TORTURETESTS))

# `make check-NAME` runs the tests in t/, tt/, and torture/ whose names start
# with NAME
PERCENT := %
.SECONDEXPANSION:
check-t/%: $(CHECKDEPS) $$(filter t/$$*$$(PERCENT),$$(TESTS))
	$(call runtests,$(filter-out $(CHECKDEPS),$^))
check-tt/%: $(CHECKDEPS) $$(filter tt/$$*$$(PERCENT),$$(SPECIALTESTS))
	$(call runtests,$(filter-out $(CHECKDEPS),$^))
check-torture/%: $(CHECKDEPS) $$(filter torture/$$*$$(PERCENT),$$(TORTURETESTS))
	$(call runtests,$(filter-out $(CHECKDEPS),$^))
check-%: $(CHECKDEPS) $$(filter $$*$$(PERCENT) t/$$*$$(PERCENT) tt/$$*$$(PERCENT) torture/$$*$$(PERCENT),$$(ALLTESTS))
	$(call runtests,$(filter-out $(CHECKDEPS),$^))

clean: clean-main
clean-main:
	$(call run,rm -f $(PROGRAMS) $(HELPERS) $(ALLTESTS) *.o t/*.o tt/*.o torture/*.o core *.core,CLEAN)
	$(call run,rm -rf $(DEPSDIR) bin out files inputs outputs stdoutputs *.dSYM)
	@if test -f /usr/bin/cs61-docker-version; then rm -rf /tmp/cs61-iotest; fi

distclean: clean

.PRECIOUS: %.o
.PHONY: all programs stdio slow syscall special clean clean-main distclean \
	check check-default check-special check-torture \
	check-% check-t/% check-tt/% check-torture/%
export CACHE STRACE NOSTDIO TRIALS MAXTIME TMP V
