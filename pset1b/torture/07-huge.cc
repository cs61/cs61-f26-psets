#include "m61.hh"
#include <cstdint>
// Check that impossible allocations fail cleanly: they return `nullptr`,
// leave active statistics alone, and don't break later allocations.

int main() {
    void* p = m61_malloc(100);
    assert(p);

    assert(m61_malloc(SIZE_MAX) == nullptr);
    assert(m61_malloc(SIZE_MAX - 200) == nullptr);
    assert(m61_malloc(SIZE_MAX / 2 + 8) == nullptr);
    assert(m61_malloc(size_t(8) << 20) == nullptr);   // `p` is still active

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 1);
    assert(stats.active_bytes == 100);

    // the allocator still works
    void* q = m61_malloc(1000);
    assert(q);
    m61_free(p);
    m61_free(q);
    p = m61_malloc(7 << 20);
    assert(p);
    m61_free(p);

    stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
