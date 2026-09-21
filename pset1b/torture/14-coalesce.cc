#include "m61.hh"
#include "utils.hh"
#include <algorithm>
// Check that adjacent freed blocks are coalesced into larger blocks.

int main() {
    // fill the buffer with 850-byte blocks
    constexpr size_t nmax = 7168;
    void* ptrs[nmax];
    for (size_t i = 0; i != nmax; ++i) {
        ptrs[i] = m61_malloc(850);
        assert(ptrs[i]);
    }

    // free all but the lowest-addressed block, in random order
    std::sort(ptrs, ptrs + nmax);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh
    size_t n = nmax;
    while (n != 1) {
        size_t i = rand.uniform(size_t(1), n - 1);
        m61_free(ptrs[i]);
        ptrs[i] = ptrs[n - 1];
        --n;
    }

    // the freed blocks together can hold one big block
    void* bigptr = m61_malloc(6091950);
    assert(bigptr);
    m61_free(bigptr);
    m61_free(ptrs[0]);

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == nmax + 1);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
