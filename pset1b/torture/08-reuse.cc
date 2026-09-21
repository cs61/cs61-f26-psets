#include "m61.hh"
// Check for memory reuse: 10,000 allocations, at most one active at a time.
// (Without reuse, the 8 MiB buffer runs out after about 8,000.)

int main() {
    for (int i = 0; i != 10000; ++i) {
        void* ptr = m61_malloc(1000);
        assert(ptr);
        m61_free(ptr);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == 10000);
    assert(stats.total_bytes == 10000000);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
