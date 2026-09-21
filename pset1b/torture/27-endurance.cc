#include "m61.hh"
#include "utils.hh"
// Torture test: 5,000,000 random allocations and frees with at most 64
// blocks active at a time. Statistics are checked at the end.

int main() {
    time_limit(20);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh

    void* ptrs[64] = {};
    size_t sizes[64] = {};
    size_t total_count = 0, total_bytes = 0;
    size_t active_count = 0, active_bytes = 0;

    for (int i = 0; i != 5000000; ++i) {
        int j = rand.uniform(0, 63);
        if (ptrs[j]) {
            m61_free(ptrs[j]);
            ptrs[j] = nullptr;
            --active_count;
            active_bytes -= sizes[j];
        } else {
            sizes[j] = rand.uniform(1, 256);
            ptrs[j] = m61_malloc(sizes[j]);
            assert(ptrs[j]);
            ++total_count;
            total_bytes += sizes[j];
            ++active_count;
            active_bytes += sizes[j];
        }
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == total_count);
    assert(stats.total_bytes == total_bytes);
    assert(stats.active_count == active_count);
    assert(stats.active_bytes == active_bytes);

    for (int j = 0; j != 64; ++j) {
        m61_free(ptrs[j]);
    }

    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
