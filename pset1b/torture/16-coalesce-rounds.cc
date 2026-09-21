#include "m61.hh"
#include "utils.hh"
#include <vector>
// Check repeated coalescing: fill the buffer with random-sized blocks,
// free them all in random order, then allocate one huge block. Repeat.

int main() {
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh
    std::vector<void*> ptrs;

    for (int round = 0; round != 20; ++round) {
        // allocate random-sized blocks until 5 MiB is active
        size_t active_bytes = 0;
        while (active_bytes < (5 << 20)) {
            size_t sz = rand.uniform(1, 4096);
            void* ptr = m61_malloc(sz);
            assert(ptr);
            ptrs.push_back(ptr);
            active_bytes += sz;
        }

        // free them in random order
        while (!ptrs.empty()) {
            size_t i = rand.uniform(size_t(0), ptrs.size() - 1);
            m61_free(ptrs[i]);
            ptrs[i] = ptrs.back();
            ptrs.pop_back();
        }

        // now nearly the whole buffer should be available
        void* bigptr = m61_malloc((size_t) (7.9 * (1 << 20)));
        assert(bigptr);
        m61_free(bigptr);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
