#include "m61.hh"
#include "utils.hh"
// Fragmentation score. Up to 20,000 small blocks (16–64 bytes) are active
// at a time and are freed in random order. Every 20th operation also
// requests a large block (64–512 KiB) and frees the previous large block,
// as a program might for a temporary buffer.
//
// Whether a large request can be satisfied depends on where the allocator
// put the small blocks: if they were carved out of freed large blocks, the
// heap may hold plenty of free space but no hole big enough. That's a
// policy choice, not a bug, so large requests are allowed to fail here.
// The test reports how many succeeded. Small requests must always succeed.

int main() {
    time_limit(20);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh

    constexpr size_t nmax = 20000;
    static void* ptrs[nmax];
    size_t n = 0;
    void* large = nullptr;
    size_t large_requests = 0, large_successes = 0;

    for (int i = 0; i != 250000; ++i) {
        if (n == nmax || (n > 0 && rand.coin_flip(0.45))) {
            // free a random small block
            size_t j = rand.uniform(size_t(0), n - 1);
            m61_free(ptrs[j]);
            --n;
            ptrs[j] = ptrs[n];
        } else {
            ptrs[n] = m61_malloc(rand.uniform(16, 64));
            assert(ptrs[n]);
            ++n;
        }

        if (i % 20 == 0) {
            void* new_large = m61_malloc(rand.uniform(64 << 10, 512 << 10));
            ++large_requests;
            if (new_large) {
                ++large_successes;
            }
            m61_free(large);
            large = new_large;
        }
    }

    for (size_t j = 0; j != n; ++j) {
        m61_free(ptrs[j]);
    }
    m61_free(large);

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);

    void* bigptr = m61_malloc((size_t) (7.9 * (1 << 20)));
    assert(bigptr);
    m61_free(bigptr);

    std::print("{}large allocations: {} of {} succeeded ({:.1f}%){}\n",
               termcolor::orange, large_successes, large_requests,
               100.0 * large_successes / large_requests, termcolor::plain);
    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
