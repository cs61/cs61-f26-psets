#include "m61.hh"
#include "utils.hh"
// Torture test for free-block search. The buffer is filled with 16-byte
// blocks and every other one is freed, leaving 40,000 small holes that
// cannot be coalesced. Then the allocator must repeatedly find room for a
// 2000-byte block. This takes a long time if every allocation examines
// every free block, so the test has a time limit.

int main() {
    time_limit(10);

    constexpr size_t n = 80000;
    static void* ptrs[n];
    for (size_t i = 0; i != n; ++i) {
        ptrs[i] = m61_malloc(16);
        assert(ptrs[i]);
    }
    for (size_t i = 0; i < n; i += 2) {
        m61_free(ptrs[i]);
    }

    for (int i = 0; i != 1000000; ++i) {
        void* ptr = m61_malloc(2000);
        assert(ptr);
        m61_free(ptr);
    }

    for (size_t i = 1; i < n; i += 2) {
        m61_free(ptrs[i]);
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == n + 1000000);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);

    void* bigptr = m61_malloc((size_t) (7.9 * (1 << 20)));
    assert(bigptr);
    m61_free(bigptr);

    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
