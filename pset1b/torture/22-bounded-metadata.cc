#include "m61.hh"
#include "utils.hh"
#include <cstring>
#include <deque>
#include <sys/resource.h>
// Check that metadata is bounded: 2,000,000 allocations with at most 100
// active at a time should not use much memory beyond the 8 MiB buffer.

static size_t memory_usage_kb() {
    struct rusage u;
    int r = getrusage(RUSAGE_SELF, &u);
    assert(r == 0);
#if __APPLE__
    return u.ru_maxrss / 1024;   // macOS reports bytes
#else
    return u.ru_maxrss;          // Linux reports KiB
#endif
}

int main() {
    time_limit(10);
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh
    size_t before = memory_usage_kb();

    std::deque<void*> ptrs;
    for (int i = 0; i != 2000000; ++i) {
        if (ptrs.size() == 100 || (!ptrs.empty() && rand.uniform(0, 2) == 0)) {
            m61_free(ptrs.front());
            ptrs.pop_front();
        } else {
            void* ptr = m61_malloc(20);
            assert(ptr);
            memset(ptr, i % 256, 10);
            ptrs.push_back(ptr);
        }
    }
    while (!ptrs.empty()) {
        m61_free(ptrs.front());
        ptrs.pop_front();
    }

    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count >= 1000000);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);

    size_t after = memory_usage_kb();
    size_t growth = after > before ? after - before : 0;
    std::print("{}memory usage grew by {} KiB, elapsed time {:.2f} s{}\n",
               termcolor::orange, growth, elapsed_time(), termcolor::plain);
#if !SAN
    // Sanitizers have memory overhead of their own, so this check only
    // runs when they're off.
    assert(growth < 16 * 1024);
#endif
}
