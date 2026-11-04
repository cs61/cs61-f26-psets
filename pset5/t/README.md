# Pset 5 tests directory

You can populate this directory with whatever default tests you’d like. Any
file named `*.cc` is treated as a default test: a program that uses
`io61_tester` (see `iotest.hh`) to run IO61 programs and check the results.
Default tests should pass with our stdio version of IO61
(`IO61=stdio t/TESTNAME`). Build and run a test with `make check-TESTNAME`, or
run a built test on the command line with a command like `t/TESTNAME`.
Exiting with status 0 means the test passed; exiting with any other status
means it failed.

Name each test after the feature it tests, as in `t/reverse-mytest.cc`, so
that `make check-reverse` runs it along with the other reverse tests.
Performance tests call `report_timing()` after checking their results.
