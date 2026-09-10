#include "m61.hh"

static char* buffer = nullptr;
static char* end_buffer = nullptr;
static char* last_allocation = nullptr;

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
    void* ptr = buffer;
    last_allocation = buffer;
    buffer += sz;
    return ptr;
}

void m61_free(void* ptr) {
    if (ptr && ptr == last_allocation) {
        buffer = last_allocation;
        last_allocation = nullptr;
    }
}

m61_statistics m61_get_statistics() {
    return m61_statistics{};
}
