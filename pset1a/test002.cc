// Test: We can allocate at least 7.9 mebibytes
#include "m61.hh"

int main() {
    void* ptr = m61_malloc((size_t) (7.9 * (1 << 20)));
}
