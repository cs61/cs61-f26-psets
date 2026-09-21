#include "m61.hh"
#include <cstring>
// Check that freed memory can be reused immediately, even when the buffer
// is otherwise full.

static void check_contents(unsigned char* p, unsigned char ch) {
    unsigned char buf[10];
    memset(buf, ch, 10);
    assert(memcmp(p, buf, 10) == 0);
}

int main() {
    // fill the buffer with 850-byte allocations
    constexpr size_t nmax = 10000;
    unsigned char* ptrs[nmax];
    size_t n = 0;
    while (n != nmax) {
        ptrs[n] = (unsigned char*) m61_malloc(850);
        if (!ptrs[n]) {
            break;
        }
        memset(ptrs[n], n & 255, 10);
        ++n;
    }
    assert(n >= 8000);   // 8 MiB / 850 bytes ≈ 9800; metadata may use some

    // free one block in the middle; it should be possible to allocate again
    size_t f = n / 2;
    check_contents(ptrs[f], f & 255);
    m61_free(ptrs[f]);
    ptrs[f] = (unsigned char*) m61_malloc(850);
    assert(ptrs[f]);
    memset(ptrs[f], f & 255, 10);

    for (size_t i = 0; i != n; ++i) {
        check_contents(ptrs[i], i & 255);
        m61_free(ptrs[i]);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
