#include "m61.hh"
#include "utils.hh"
#include <cstring>
#include <vector>
// Check that error detection still works when the heap is busy. Each
// check first runs a random workload, then makes a mistake with one of
// the active blocks: a double free, a wild free, or a wild write.

struct block {
    char* ptr;
    size_t size;
};

// Run a random workload with up to 5,000 active blocks of 16–200 bytes,
// then return a random active block.
static block busy_heap() {
    random_source rand;
    rand.env_seed();   // uses M61_SEED; see utils.hh
    std::vector<block> live;

    for (int i = 0; i != 200000; ++i) {
        if (live.size() == 5000 || (!live.empty() && rand.coin_flip(0.45))) {
            size_t j = rand.uniform(size_t(0), live.size() - 1);
            m61_free(live[j].ptr);
            live[j] = live.back();
            live.pop_back();
        } else {
            block b;
            b.size = rand.uniform(16, 200);
            b.ptr = (char*) m61_malloc(b.size);
            if (!b.ptr) {
                // Not `assert`: this runs in a subtest, and a failed
                // assertion would look like a detected memory error.
                fprintf(stderr, "allocation failed\n");
                exit(2);
            }
            live.push_back(b);
        }
    }

    return live[rand.uniform(size_t(0), live.size() - 1)];
}

static void double_free() {
    block b = busy_heap();
    m61_free(b.ptr);
    m61_free(b.ptr);
}

static void wild_free() {
    block b = busy_heap();
    m61_free(b.ptr + 8);
}

static void wild_write() {
    block b = busy_heap();
    memset(b.ptr + b.size, 0, 8);
    m61_free(b.ptr);
}

int main() {
    time_limit(20);

    subtest_result r = run_subtest(double_free);
    r.print("double free in busy heap");
    assert(r.aborted());   // not exited: the workload itself must succeed

    r = run_subtest(wild_free);
    r.print("wild free in busy heap");
    assert(r.aborted());

    r = run_subtest(wild_write);
    r.print("wild write in busy heap");
    assert(r.aborted());

    std::print("{}elapsed time {:.2f} s{}\n",
               termcolor::orange, elapsed_time(), termcolor::plain);
}
