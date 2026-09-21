#include "m61.hh"
#include "utils.hh"
#include <deque>
// Check reuse and coalescing under two common allocation patterns: a
// queue, where the oldest block is freed first, and a stack, where the
// newest block is freed first. Up to 10,000 random-sized blocks are active
// at a time. Every block's contents are checked when it is freed.

struct block {
    char* ptr;
    size_t size;
    unsigned id;     // determines the block's contents
};

static void run(random_source& rand, bool free_oldest) {
    std::deque<block> live;
    unsigned next_id = 0;

    for (int i = 0; i != 1500000; ++i) {
        if (live.size() == 10000 || (!live.empty() && rand.coin_flip(0.45))) {
            block b = free_oldest ? live.front() : live.back();
            if (free_oldest) {
                live.pop_front();
            } else {
                live.pop_back();
            }
            assert(check_pattern(b.ptr, b.size, b.id));
            m61_free(b.ptr);
        } else {
            block b;
            b.size = rand.uniform(1, 512);
            b.id = next_id++;
            b.ptr = (char*) m61_malloc(b.size);
            assert(b.ptr);
            fill_pattern(b.ptr, b.size, b.id);
            live.push_back(b);
        }
    }

    for (block& b : live) {
        assert(check_pattern(b.ptr, b.size, b.id));
        m61_free(b.ptr);
    }

    // everything was freed, so nearly the whole buffer should be available
    void* bigptr = m61_malloc((size_t) (7.9 * (1 << 20)));
    assert(bigptr);
    m61_free(bigptr);
}

int main() {
    time_limit(20);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh

    run(rand, true);     // queue
    run(rand, false);    // stack

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);

    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
