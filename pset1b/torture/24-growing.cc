#include "m61.hh"
#include "utils.hh"
// Torture test with large blocks. Eight "arrays" repeatedly double in
// size, the way a growing std::vector does: each doubling allocates a new
// block and then frees the old one. Arrays that reach 256 KiB are freed
// and start over. Every block's contents are checked when it is freed.

struct block {
    char* ptr = nullptr;
    size_t size = 0;
    unsigned id = 0;     // determines the block's contents
};

int main() {
    time_limit(10);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh

    block arrays[8];
    unsigned next_id = 0;

    for (int i = 0; i != 20000; ++i) {
        block& a = arrays[rand.uniform(0, 7)];
        size_t newsize = a.size == 0 ? 16 : a.size * 2;
        if (newsize > (256 << 10)) {
            // too big: free it and start over
            assert(check_pattern(a.ptr, a.size, a.id));
            m61_free(a.ptr);
            a = block();
            continue;
        }

        block bigger;
        bigger.size = newsize;
        bigger.id = next_id++;
        bigger.ptr = (char*) m61_malloc(newsize);
        assert(bigger.ptr);
        fill_pattern(bigger.ptr, newsize, bigger.id);

        if (a.ptr) {
            assert(check_pattern(a.ptr, a.size, a.id));
            m61_free(a.ptr);
        }
        a = bigger;
    }

    for (block& a : arrays) {
        if (a.ptr) {
            assert(check_pattern(a.ptr, a.size, a.id));
            m61_free(a.ptr);
        }
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);

    void* bigptr = m61_malloc((size_t) (7.9 * (1 << 20)));
    assert(bigptr);
    m61_free(bigptr);

    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
