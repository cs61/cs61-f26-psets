#include "m61.hh"
#include "utils.hh"
// Check for memory reuse: up to 100 variable-sized active allocations,
// allocated and freed in random order.

int main() {
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh

    constexpr size_t nmax = 100;
    char* ptrs[nmax];
    size_t sizes[nmax];
    size_t n = 0;
    size_t total_count = 0, total_bytes = 0;

    // 10000 times, either allocate a new block or free a random block
    for (int i = 0; i != 10000; ++i) {
        if (rand.uniform(size_t(0), nmax * nmax - 1) < n * n) {
            // free a random block
            size_t j = rand.uniform(size_t(0), n - 1);
            m61_free(ptrs[j]);
            --n;
            ptrs[j] = ptrs[n];
            sizes[j] = sizes[n];
        } else {
            // allocate a new block; check it doesn't overlap active blocks
            size_t size = rand.uniform(1, 4000);
            char* ptr = (char*) m61_malloc(size);
            assert(ptr);
            for (size_t k = 0; k != n; ++k) {
                assert(ptrs[k] + sizes[k] <= ptr || ptr + size <= ptrs[k]);
            }
            ptrs[n] = ptr;
            sizes[n] = size;
            ++n;
            ++total_count;
            total_bytes += size;
        }
    }

    for (size_t j = 0; j != n; ++j) {
        m61_free(ptrs[j]);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == total_count);
    assert(stats.total_bytes == total_bytes);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
