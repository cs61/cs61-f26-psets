#include "m61.hh"
#include "utils.hh"
// Check allocation speed: 2,500,000 allocations, at most 5 active at a
// time. An allocator with internal metadata should finish this in well
// under a second.

int main() {
    time_limit(20);

    for (int i = 0; i != 500000; ++i) {
        void* ptrs[5];
        for (int j = 0; j != 5; ++j) {
            ptrs[j] = m61_malloc(1000);
            assert(ptrs[j]);
        }
        for (int j = 5; j != 0; --j) {
            m61_free(ptrs[j - 1]);
        }
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == 2500000);
    assert(stats.total_bytes == 2500000000);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);

    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
