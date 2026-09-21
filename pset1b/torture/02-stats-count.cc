#include "m61.hh"
// Check allocation count and size statistics.

int main() {
    void* ptrs[10];
    for (int i = 0; i != 10; ++i) {
        ptrs[i] = m61_malloc(i + 1);
        assert(ptrs[i]);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == 10);
    assert(stats.total_bytes == 55);
    assert(stats.active_count == 10);
    assert(stats.active_bytes == 55);

    for (int i = 0; i != 5; ++i) {
        m61_free(ptrs[i]);
    }

    stats = m61_get_statistics();
    assert(stats.total_count == 10);
    assert(stats.total_bytes == 55);
    assert(stats.active_count == 5);
    assert(stats.active_bytes == 40);   // 6 + 7 + 8 + 9 + 10

    for (int i = 5; i != 10; ++i) {
        m61_free(ptrs[i]);
    }

    stats = m61_get_statistics();
    assert(stats.total_count == 10);
    assert(stats.total_bytes == 55);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
