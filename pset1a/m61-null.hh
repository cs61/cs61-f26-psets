#ifndef M61_NULL_HH
#define M61_NULL_HH
#include <cstdlib>
#include <cassert>

void* m61_malloc(size_t) {
    return nullptr;
}

void m61_free(void*) {
}

#endif
