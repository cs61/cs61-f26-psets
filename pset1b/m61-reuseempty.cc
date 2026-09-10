#include "m61.hh"
#include <map>

static char* buffer = nullptr;
static char* end_buffer = nullptr;
static m61_statistics gs;

void* m61_malloc(size_t sz) {
    if (!buffer) {
        buffer = reinterpret_cast<char*>(malloc(8 << 20));
        assert(buffer);
        end_buffer = buffer + (8 << 20);
    }

    size_t space = end_buffer - buffer;
    if (sz > space) {
        // out of memory
        return nullptr;
    }

    void* ptr = buffer;
    gs.total_bytes += sz;
    gs.total_count += 1;
    gs.active_bytes += sz;
    gs.active_count += 1;
    buffer += sz;
    return ptr;
}

void m61_free(void* ptr) {
    if (ptr) {
        --gs.active_count;
        if (gs.active_count == 0) {
            buffer = end_buffer - (8 << 20);
            gs.active_bytes = 0;
        }
    }
}

m61_statistics m61_get_statistics() {
    return gs;
}
