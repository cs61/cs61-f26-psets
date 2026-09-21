#include "m61.hh"
#include <cstring>
// Check that active allocations do not overlap, and that their contents
// are preserved by later allocations and frees.

const char* const contents[10] = {
    "",
    "M",
    "ar",
    "ria",
    "ge, ",
    "by Ma",
    "rianne",
    " Moore.",
    " This in",
    "stitution"
};

static void check_pointers(char** ptrs) {
    for (int i = 0; i != 10; ++i) {
        if (ptrs[i]) {
            // contents preserved
            assert(memcmp(ptrs[i], contents[i], i) == 0);
            // no overlap with any other active allocation
            for (int j = 0; j != 10; ++j) {
                if (i != j && ptrs[j]) {
                    assert(ptrs[i] + i <= ptrs[j] || ptrs[j] + j <= ptrs[i]);
                }
            }
        }
    }
}

int main() {
    char* ptrs[10] = {};
    for (int i = 0; i != 10; ++i) {
        ptrs[i] = (char*) m61_malloc(i + 1);
        assert(ptrs[i]);
        memcpy(ptrs[i], contents[i], i);
        check_pointers(ptrs);
    }
    for (int i = 0; i != 5; ++i) {
        m61_free(ptrs[i]);
        ptrs[i] = nullptr;
        check_pointers(ptrs);
    }
    for (int i = 0; i != 5; ++i) {
        ptrs[i] = (char*) m61_malloc(i + 1);
        assert(ptrs[i]);
        memcpy(ptrs[i], contents[i], i);
        check_pointers(ptrs);
    }
}
