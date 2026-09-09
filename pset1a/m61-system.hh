#ifndef M61_SYSTEM_HH
#define M61_SYSTEM_HH
#include <cstdlib>
#include <cassert>

void* m61_malloc(size_t n) {
    return malloc(n);
}

void m61_free(void* ptr) {
    free(ptr);
}

#endif
