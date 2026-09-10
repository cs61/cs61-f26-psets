#include "m61.hh"

// Initially a copy of a version of m61-once.cc
// (without alignment fixes)

static char* buffer = nullptr;
static char* end_buffer = nullptr;

void* m61_malloc(size_t sz) {
    if (!buffer) {
        buffer = reinterpret_cast<char*>(malloc(8 << 20));
        assert(buffer);
        end_buffer = buffer + (8 << 20);
    }

    size_t space = end_buffer - buffer;
    if (sz > space) {
        // out of memory in buffer
        return nullptr;
    }

    // successful allocation
    void* ptr = buffer;
    buffer += sz;
    return ptr;
}

void m61_free(void*) {
}

m61_statistics m61_get_statistics() {
    return m61_statistics{};
}
