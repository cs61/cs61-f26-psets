#ifndef M61_ONCE_HH
#define M61_ONCE_HH
#include <cstdlib>
#include <cassert>

void* m61_malloc(size_t sz) {
    static char* buffer = nullptr;
    static size_t left = 0;

    if (!buffer) {
        buffer = malloc(8 << 20);
        assert(buffer);
        left = 8 << 20;
    }

    if (sz > left) {
        // out of memory in buffer
        return nullptr;
    }
    void* ptr = buffer;
    buffer += sz;
    return ptr;
}

void m61_free(void*) {
}

#endif
