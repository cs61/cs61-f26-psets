# Pset 4 tests directory

You can populate this directory with whatever default tests you’d like. Any
file named `*.cc` is treated as a default test: a program that uses
`sh61_tester` (see `shtest.hh`) to run a shell and check the results. Default
tests should pass with bash (`SHTEST_SHELL=bash t/TESTNAME`). Build and run a
test with `make check-TESTNAME`, or run a built test on the command line with a
command like `t/TESTNAME`. Exiting with status 0 means the test passed; exiting
with any other status means it failed.

Name each test after the feature it tests (`simple`, `list`, `cond`, `pipe`,
`bg`, `zombie`, `redir`, or `cd`), as in `t/cond-mytest.cc`, so that
`make check-cond` runs it along with the other conditional tests.
