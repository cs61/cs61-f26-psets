# Pset 1 torture tests

These tests are ours. They cover statistics and basics (01–07), memory reuse
(08–13), coalescing (14–16), and error detection (17–20). Then come some
performance workloads: faster allocators will run the tests in less time. There
are some general workloads (21–27), workloads that depend on how free blocks are
found (28–29), a fragmentation score (30), space efficiency (31), and error
detection under load (32). Finally there’s a diabolical invalid free (33). Run
one test with `make check-torture/NAME` and all of them with `make
check-torture`.

Error-detection tests run the erroneous code in a subtest (see `run_subtest` in
`utils.hh`) and expect the allocator to print a message to standard error and
then call `abort()` or `exit(1)`. The tests don't check the message’s wording,
but they print it, so you can see how helpful it is.

Tests that use randomness seed it from the `M61_SEED` environment variable (see
`env_seed` in `utils.hh`). This defaults to a fixed number (61), which makes
tests repeatable. Use `make M61_SEED= check-torture/24` to run with a random
seed.

Performance tests have time limits (see `time_limit` in `utils.hh`). The limit
is four times longer when sanitizers are on. Setting the `M61_TIME_LIMIT`
environment variable overrides the limit, and `M61_TIME_LIMIT=0` disables it.
