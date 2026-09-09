// Test: Two `m61_malloc(0)` calls return distinct nonnull pointers
#include "m61.hh"

int main() {
    void* ptr1 = m61_malloc(0);
    void* ptr2 = m61_malloc(0);
    assert(ptr1 && ptr2);
    assert(ptr1 != ptr2);
}
