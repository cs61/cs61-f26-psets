# Pset 4 torture tests

These tests are ours. Like the tests in `t/`, they are named after the feature
they test. Run one test with `make check-torture/NAME` and all of them with
`make check-torture`; `make check-NAME` runs the matching tests in `t/`,
`tt/`, and `torture/` together.

Some background tests check timing or the shell’s processes while the shell
is running, so they take some time. All the tests pass with bash
(`SHTEST_SHELL=bash make check-torture`).
