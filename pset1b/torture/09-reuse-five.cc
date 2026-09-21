#include "m61.hh"
// Check for memory reuse: at most 5 active allocations, which must not
// overlap.

int main() {
    for (int i = 0; i != 10000; ++i) {
        char* ptrs[5];
        for (int j = 0; j != 5; ++j) {
            ptrs[j] = (char*) m61_malloc(1000);
            assert(ptrs[j]);
            for (int k = 0; k != j; ++k) {
                assert(ptrs[k] + 1000 <= ptrs[j] || ptrs[j] + 1000 <= ptrs[k]);
            }
        }
        for (int j = 5; j != 0; --j) {
            m61_free(ptrs[j - 1]);
        }
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == 50000);
    assert(stats.total_bytes == 50000000);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
