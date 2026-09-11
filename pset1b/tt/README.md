# Pset 1b special tests directory

You can populate this directory with whatever *special* tests you’d
like. These tests need not run against any M61 allocator (but they
should run against `m61-pset.cc`). For instance, they might connect
to special “test seam” functions provided by your allocator to gain
access to internal allocator state. Build special tests with
`make special` or `make tt/TESTNAME`.
