all:
	@echo '*** Build a specific test with `make testNNN`' 1>&2
	exit 1

test%: always
	c++ -o $@ -g -fsanitize=undefined test$*.cc

always:

clean:
	rm -f test[0-9] test[0-9][0-9] test[0-9][0-9][0-9]

.PHONY: all always clean
