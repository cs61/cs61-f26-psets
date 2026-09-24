#include "m61.hh"
#include <map>

static m61_statistics gs;         // global statistics

// Idea: Track ALL blocks, allocated or free, in a single structure!
// Each block says whether it's free.
struct allocinfo {
    size_t bsz;      // size of block (includes padding)
    size_t usz;      // user size of block (only meaningful on allocated blocks)
    bool free;
};
static std::map<char*, allocinfo> blocks;

void* m61_malloc(size_t sz) {
    if (blocks.empty()) {
        char* buffer = reinterpret_cast<char*>(malloc(8 << 20));
        assert(buffer);
        // whole buffer is initially free
        blocks.insert({ buffer, { 8 << 20, 0, true }});
    }

    auto it = blocks.begin();
    while (it != blocks.end() && (!it->second.free || it->second.bsz < sz)) {
        ++it;
    }
    if (it == blocks.end()) {
        return nullptr;
    }

    // Successful allocation! Current state:
    //
    //     `it`
    //       ↓
    //       ┌──────────────────────────────────────────────┐
    //       │     first:   ║           second:             │
    //       │ ptr to first ║   bsz   │    usz   │   free   │
    //   <---│ byte in free ║ size of │  size of │          │----->
    //       │     space    ║  block  │user alloc│          │
    //       ├──────────────────────────────────────────────┤
    //       │      PTR     ║   BSZ   │    -     │   true   │
    //       └──────────────────────────────────────────────┘
    //
    // where BSZ ≥ the user’s requested size SZ.
    //
    // So split off an aligned block big enough to contain SZ, and mark it
    // allocated. The rest of the original block remains free. (ASZ is a
    // multiple of 16 so that SZ ≤ ASZ ≤ BSZ.)
    //
    //   `it`
    //     ↓
    //     ┌───────────────────────────────┐    ┌─────────────────────────────┐
    //  <--│  PTR  ║  ASZ  │  SZ   │ false │<-->│PTR+ASZ║BSZ-ASZ│  -  │  true │--->
    //     └───────────────────────────────┘    └─────────────────────────────┘

    // alignment: round up to multiple of 16
    size_t asz = sz && sz % 16 == 0 ? sz : (sz | 15) + 1;

    // split this block!
    // first insert new block
    char* nextptr = it->first + asz;
    blocks[nextptr] = { it->second.bsz - asz, 0, true };
    // then shrink our portion & mark it allocated
    it->second.bsz = asz;
    it->second.usz = sz;
    it->second.free = false;

    // statistics
    gs.total_count += 1;
    gs.total_bytes += sz;
    gs.active_count += 1;
    gs.active_bytes += sz;

    return it->first;
}

void m61_free(void* ptr) {
    if (!ptr) {
        return;
    }
    char* bufptr = reinterpret_cast<char*>(ptr);
    auto it = blocks.find(bufptr);
    if (it == blocks.end()) {
        assert(false && "invalid free");
    } else if (it->second.free) {
        assert(false && "double free");
    }
    it->second.free = 1;

    // statistics
    gs.active_count -= 1;
    gs.active_bytes -= it->second.usz;
}

m61_statistics m61_get_statistics() {
    return gs;
}
