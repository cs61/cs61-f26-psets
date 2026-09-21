#include "m61.hh"
#include <cstdint>
// Check that a large freed block can be split into many smaller pieces.

int main() {
    void* bigptr = m61_malloc(7 << 20);
    assert(bigptr);
    void* smallptr = m61_malloc(1000);
    assert(smallptr);

    uintptr_t bigaddr = (uintptr_t) bigptr;
    uintptr_t smalladdr = (uintptr_t) smallptr;
    assert(bigaddr + (7 << 20) <= smalladdr || smalladdr + 1000 <= bigaddr);

    m61_free(bigptr);

    // the 7 MiB should now hold at least 7168 850-byte blocks
    constexpr size_t nmax = 7168;
    void* ptrs[nmax];
    for (size_t i = 0; i != nmax; ++i) {
        ptrs[i] = m61_malloc(850);
        assert(ptrs[i]);
    }

    for (size_t i = 0; i != nmax; ++i) {
        m61_free(ptrs[i]);
    }
    m61_free(smallptr);

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == 7170);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
