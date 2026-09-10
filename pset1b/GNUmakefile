TESTS = $(patsubst %.cc,%,$(sort $(wildcard *[0-9][0-9][0-9].cc)))

all: $(TESTS)

-include build/rules.mk

%.o: %.cc $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) -o $@ -c,COMPILE,$<)

m61.o: m61-$(M61).cc $(M61STAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) -o $@ -c,COMPILE,$<)

$(TESTS): %: %.o m61.o $(OBJS)
	$(call run,$(CXX) $(CXXFLAGS) $(LDFLAGS) $(O) -o $@ $^ $(LIBS),LINK $@)

clean: clean-main
clean-main:
	$(call run,rm -f $(TESTS) *.o core *.core,CLEAN)
	$(call run,rm -rf out *.dSYM $(DEPSDIR))

distclean: clean

.PRECIOUS: %.o
.PHONY: all always clean clean-main clean-hook distclean
