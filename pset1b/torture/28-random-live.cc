#include "m61.hh"
#include "utils.hh"
#include <algorithm>
#include <vector>
// Torture test: 5,000,000 random allocations and frees with up to 10,000
// random-sized blocks (0–160 bytes) active at a time. Every block's
// contents are checked when it is freed, and every so often all active
// blocks are checked for overlap.

struct block {
    char* ptr;
    size_t size;
    unsigned id;     // determines the block's contents
};

static void check_no_overlap(std::vector<block> blocks) {
    std::sort(blocks.begin(), blocks.end(),
              [] (const block& a, const block& b) { return a.ptr < b.ptr; });
    for (size_t i = 1; i < blocks.size(); ++i) {
        assert(blocks[i - 1].ptr < blocks[i].ptr);   // distinct, even if size 0
        assert(blocks[i - 1].ptr + blocks[i - 1].size <= blocks[i].ptr);
    }
    // everything should come from a single 8 MiB buffer
    if (!blocks.empty()) {
        char* lo = blocks.front().ptr;
        char* hi = blocks.back().ptr + blocks.back().size;
        assert(hi - lo <= (8 << 20));
    }
}

int main() {
    time_limit(20);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh

    std::vector<block> live;
    unsigned next_id = 0;
    size_t total_count = 0, total_bytes = 0;

    for (int i = 0; i != 5000000; ++i) {
        if (live.size() == 10000 || (!live.empty() && rand.coin_flip(0.45))) {
            // free a random block
            size_t j = rand.uniform(size_t(0), live.size() - 1);
            block b = live[j];
            live[j] = live.back();
            live.pop_back();
            assert(check_pattern(b.ptr, b.size, b.id));
            m61_free(b.ptr);
        } else {
            // allocate a new block
            block b;
            b.size = rand.uniform(0, 160);
            b.id = next_id++;
            b.ptr = (char*) m61_malloc(b.size);
            assert(b.ptr);
            fill_pattern(b.ptr, b.size, b.id);
            live.push_back(b);
            ++total_count;
            total_bytes += b.size;
        }
        if (i % 1000000 == 999999) {
            check_no_overlap(live);
        }
    }

    for (block& b : live) {
        assert(check_pattern(b.ptr, b.size, b.id));
        m61_free(b.ptr);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == total_count);
    assert(stats.total_bytes == total_bytes);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);

    void* bigptr = m61_malloc((size_t) (7.9 * (1 << 20)));
    assert(bigptr);
    m61_free(bigptr);

    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
