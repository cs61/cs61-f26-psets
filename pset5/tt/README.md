# Pset 5 special tests directory

You can populate this directory with whatever *special* tests you’d like.
Special tests need not pass with stdio; they might, for instance, check
behavior specific to your IO61 library. `make check` doesn’t run them. Build
special tests with `make special` or `make tt/TESTNAME`, and run them with
`make check-special` or `make check-TESTNAME`.
