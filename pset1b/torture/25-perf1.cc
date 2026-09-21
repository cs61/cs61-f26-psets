#include "m61.hh"
#include "utils.hh"
// Performance test with a single allocation size: 48-byte blocks, as for
// the nodes of a linked list. Up to 50,000 are active at a time, and they
// are freed in random order. Freed blocks should be reused for later
// allocations.

int main() {
    time_limit(20);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh

    constexpr size_t nmax = 50000;
    static void* ptrs[nmax];
    size_t n = 0;

    for (int i = 0; i != 3000000; ++i) {
        if (n == nmax || (n > 0 && rand.coin_flip(0.45))) {
            // free a random block
            size_t j = rand.uniform(size_t(0), n - 1);
            m61_free(ptrs[j]);
            --n;
            ptrs[j] = ptrs[n];
        } else {
            ptrs[n] = m61_malloc(48);
            assert(ptrs[n]);
            ++n;
        }
    }

    for (size_t j = 0; j != n; ++j) {
        m61_free(ptrs[j]);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);

    void* bigptr = m61_malloc((size_t) (7.9 * (1 << 20)));
    assert(bigptr);
    m61_free(bigptr);

    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
