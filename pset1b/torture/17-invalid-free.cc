#include "m61.hh"
#include "utils.hh"
// Check that invalid frees of non-heap pointers are detected.
//
// A detected error should print a message to standard error and then call
// `abort()`. Each bad free runs in a subtest (see `run_subtest` in
// utils.hh) so this test can check the result. The test prints the
// allocator's messages so you can see how helpful they are.

static int global;

static void free_before_heap() {
    // a pointer just before the first allocation
    char* ptr = (char*) m61_malloc(32);
    m61_free(ptr - 32);
}

static void free_small_number() {
    // a pointer nowhere near the heap
    m61_free((void*) 16);
}

static void free_stack_pointer() {
    int x = 0;
    m61_free(&x);
}

static void free_global_pointer() {
    m61_free(&global);
}

static void free_small_number_with_active_allocations() {
    void* ptrs[10];
    for (int i = 0; i != 10; ++i) {
        ptrs[i] = m61_malloc(i + 1);
    }
    for (int i = 0; i != 4; ++i) {
        m61_free(ptrs[i]);
    }
    m61_free((void*) 16);
    m61_free(ptrs[4]);
}

int main() {
    subtest_result r = run_subtest(free_before_heap);
    r.print("free of pointer before heap");
    assert(r.aborted());

    r = run_subtest(free_small_number);
    r.print("free of small number");
    assert(r.aborted());

    r = run_subtest(free_stack_pointer);
    r.print("free of stack pointer");
    assert(r.aborted());

    r = run_subtest(free_global_pointer);
    r.print("free of global pointer");
    assert(r.aborted());

    r = run_subtest(free_small_number_with_active_allocations);
    r.print("free of small number with active allocations");
    assert(r.aborted());
}
