TESTS = $(patsubst %.cc,%,$(sort $(wildcard test*.cc)))

all:
	@echo '*** Build a specific test with `make testNNN`' 1>&2
	exit 1

-include build/rules.mk

%.o: %.cc $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) -o $@ -c,COMPILE,$<)

test%: test%.o $(OBJS)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)

clean: clean-main
clean-main:
	$(call run,rm -f $(TESTS) *.o core *.core,CLEAN)
	$(call run,rm -rf out *.dSYM $(DEPSDIR))

distclean: clean

.PRECIOUS: %.o
.PHONY: all always clean clean-main clean-hook distclean
