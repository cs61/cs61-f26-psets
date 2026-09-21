#include "m61.hh"
#include "utils.hh"
// Check that every byte of an allocation can be used: writing all `sz`
// bytes of an `m61_malloc(sz)` block must not disturb other allocations
// or be reported as an error.

int main() {
    constexpr int nptrs = 300;
    char* ptrs[nptrs];

    // allocate blocks of size 0, 1, 2, ..., 299 and fill each one
    for (int i = 0; i != nptrs; ++i) {
        ptrs[i] = (char*) m61_malloc(i);
        assert(ptrs[i]);
        assert(i == 0 || ptrs[i] != ptrs[i - 1]);
        fill_pattern(ptrs[i], i, i);
    }

    // free every third block; the others must be intact
    for (int i = 0; i < nptrs; i += 3) {
        m61_free(ptrs[i]);
        ptrs[i] = nullptr;
    }
    for (int i = 0; i != nptrs; ++i) {
        if (ptrs[i]) {
            assert(check_pattern(ptrs[i], i, i));
        }
    }

    // allocate and fill more blocks; the old ones must still be intact
    char* more[nptrs];
    for (int i = 0; i != nptrs; ++i) {
        more[i] = (char*) m61_malloc(i);
        assert(more[i]);
        fill_pattern(more[i], i, 1000 + i);
    }
    for (int i = 0; i != nptrs; ++i) {
        if (ptrs[i]) {
            assert(check_pattern(ptrs[i], i, i));
            m61_free(ptrs[i]);
        }
        assert(check_pattern(more[i], i, 1000 + i));
        m61_free(more[i]);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
