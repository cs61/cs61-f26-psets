#include "m61.hh"
#include <cstddef>
#include <cstdint>
// Check that every allocation, including reused memory, is aligned for
// any object type.

static bool aligned(void* ptr) {
    return (uintptr_t) ptr % alignof(std::max_align_t) == 0;
}

int main() {
    constexpr int nptrs = 200;
    void* ptrs[nptrs];
    for (int i = 0; i != nptrs; ++i) {
        ptrs[i] = m61_malloc(i);
        assert(ptrs[i]);
        assert(aligned(ptrs[i]));
    }

    // free odd-sized blocks, then reallocate them with different sizes;
    // reused memory must be aligned too
    for (int i = 1; i < nptrs; i += 2) {
        m61_free(ptrs[i]);
    }
    for (int i = 1; i < nptrs; i += 2) {
        ptrs[i] = m61_malloc(i / 3);
        assert(ptrs[i]);
        assert(aligned(ptrs[i]));
    }

    for (int i = 0; i != nptrs; ++i) {
        m61_free(ptrs[i]);
    }
}
