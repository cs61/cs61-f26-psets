#include "m61.hh"
#include "utils.hh"
#include <cstring>
// Check detection of a diabolical invalid free: the bytes surrounding a
// real allocation, including any metadata stored next to it, are copied
// into another allocation, and then a pointer into the copy is freed.

static void free_copy_of_allocation() {
    char* a = (char*) m61_malloc(208);
    char* b = (char*) m61_malloc(50);
    char* c = (char*) m61_malloc(208);
    char* p = (char*) m61_malloc(3000);
    (void) a, (void) c;

    // copy the memory around `b` into `p`
    memcpy(p, b - 208, 450);

    // `p + 208` now looks a lot like `b`, but it is not an allocation
    m61_free(p + 208);
}

int main() {
    subtest_result r = run_subtest(free_copy_of_allocation);
    r.print("free of a copy of an allocation");
    assert(r.aborted());
}
