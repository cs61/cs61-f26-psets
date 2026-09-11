#include "m61.hh"
#include <map>

static m61_statistics gs;         // global statistics
static char* buffer = nullptr;
static char* end_buffer = nullptr;
static std::map<char*, size_t> active_allocs; // size of each active allocation

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
    gs.total_count += 1;
    gs.total_bytes += sz;
    gs.active_count += 1;
    gs.active_bytes += sz;
    active_allocs.insert({ buffer, sz });

    buffer += sz;
    return ptr;
}

void m61_free(void* ptr) {
    if (ptr) {
        char* bufptr = reinterpret_cast<char*>(ptr);
        auto it = active_allocs.find(bufptr);
        if (it == active_allocs.end()) {
            assert(false && "invalid free");
        }
        gs.active_count -= 1;
        gs.active_bytes -= it->second;
        active_allocs.erase(it);
    }
}

m61_statistics m61_get_statistics() {
    return gs;
}
