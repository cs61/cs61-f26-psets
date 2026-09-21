#include "m61.hh"
#include "utils.hh"
// Performance test with power-of-two sizes: every block is 16, 32, 64,
// ..., or 4096 bytes. Up to 4,000 are active at a time, and they are freed
// in random order.

int main() {
    time_limit(20);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh

    constexpr size_t nmax = 4000;
    static void* ptrs[nmax];
    size_t n = 0;

    for (int i = 0; i != 1000000; ++i) {
        if (n == nmax || (n > 0 && rand.coin_flip(0.45))) {
            // free a random block
            size_t j = rand.uniform(size_t(0), n - 1);
            m61_free(ptrs[j]);
            --n;
            ptrs[j] = ptrs[n];
        } else {
            size_t size = size_t(1) << rand.uniform(4, 12);
            ptrs[n] = m61_malloc(size);
            assert(ptrs[n]);
            ++n;
        }
    }

    for (size_t j = 0; j != n; ++j) {
        m61_free(ptrs[j]);
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
