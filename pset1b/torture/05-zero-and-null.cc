#include "m61.hh"
// Check boundary cases: `m61_malloc(0)` returns distinct non-null pointers,
// and `m61_free(nullptr)` does nothing.

int main() {
    void* a = m61_malloc(0);
    void* b = m61_malloc(0);
    assert(a && b);
    assert(a != b);

    m61_free(nullptr);
    m61_free(a);
    m61_free(nullptr);
    m61_free(b);

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == 2);
    assert(stats.total_bytes == 0);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
