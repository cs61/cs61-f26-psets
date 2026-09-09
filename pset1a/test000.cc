// Test: `m61_malloc(0)` returns a nonnull pointer like it’s supposed to
#include "m61.hh"

int main() {
    void* ptr = m61_malloc(0);
    assert(ptr != nullptr); // crash the program here unless `ptr != nullptr`
    assert(ptr); // This means the same thing!
}
