#include "m61.hh"
#include "utils.hh"
#include <vector>
// Check space efficiency: how many blocks of a given size fit in the
// 8 MiB buffer? Metadata and alignment padding cost space; this test
// measures how much. It also reports the average number of
// bytes of buffer space used per block beyond the requested size. Two
// caveats: for very large blocks, that number mostly reflects the space
// left over at the end of the buffer; and it counts only space inside the
// buffer, so external metadata (such as a std::map) doesn't show up here.

static void check_capacity(size_t size, size_t expected) {
    // allocate blocks until allocation fails
    std::vector<void*> ptrs;
    while (ptrs.size() != 1000000) {
        void* ptr = m61_malloc(size);
        if (!ptr) {
            break;
        }
        ptrs.push_back(ptr);
    }
    size_t count = ptrs.size();
    double overhead = (double) ((8 << 20) - count * size) / count;
    std::print("{}{:7}-byte blocks: {:6} fit (need at least {}), "
               "{:.1f} bytes overhead per block{}\n",
               termcolor::orange, size, count, expected, overhead,
               termcolor::plain);
    assert(count >= expected);

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == ptrs.size());
    assert(stats.active_bytes == ptrs.size() * size);

    // free them in random order
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh
    while (!ptrs.empty()) {
        size_t i = rand.uniform(size_t(0), ptrs.size() - 1);
        m61_free(ptrs[i]);
        ptrs[i] = ptrs.back();
        ptrs.pop_back();
    }

    // nearly the whole buffer should be available again
    void* bigptr = m61_malloc((size_t) (7.9 * (1 << 20)));
    assert(bigptr);
    m61_free(bigptr);
}

int main() {
    time_limit(20);
    check_capacity(1 << 20, 7);
    check_capacity(100000, 80);
    check_capacity(1000, 7800);
    check_capacity(16, 100000);
    check_capacity(0, 100000);
}
