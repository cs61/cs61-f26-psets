#include "m61.hh"
#include "utils.hh"
// Check coalescing in the simplest three-block case: free two blocks
// that surround a third, then free the middle one, then ask for space
// that only exists if all three were merged.

int main() {
    time_limit(10);   // a corrupted free list can make `m61_malloc` loop

    char* a = (char*) m61_malloc(1000);
    char* b = (char*) m61_malloc(1000);
    char* c = (char*) m61_malloc(1000);
    char* d = (char*) m61_malloc(1000);   // keeps `c` away from untouched memory
    assert(a && b && c && d);

    m61_free(a);
    m61_free(c);
    m61_free(b);

    // `a`, `b`, and `c` together should hold a 3000-byte block
    char* big = (char*) m61_malloc(3000);
    assert(big);
    assert(big + 3000 <= d || d + 1000 <= big);   // no overlap with `d`
    m61_free(big);
    m61_free(d);

    // and now everything should be free
    big = (char*) m61_malloc((size_t) (7.9 * (1 << 20)));
    assert(big);
    m61_free(big);

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
