#include "m61.hh"
#include "utils.hh"
// Check that frees of pointers inside the heap, but not at the start of
// an active allocation, are detected.

static void free_interior_pointer() {
    char* ptr = (char*) m61_malloc(2001);
    m61_free(ptr + 128);
}

static void free_misaligned_pointer() {
    // this must be reported in an orderly way, not by crashing
    char* ptr = (char*) m61_malloc(2001);
    m61_free(ptr + 127);
}

static void free_pointer_into_freed_allocation() {
    // an invalid free, not a double free
    char* ptr = (char*) m61_malloc(2001);
    m61_free(ptr);
    m61_free(ptr + 150);
}

static void free_interior_pointer_with_active_neighbors() {
    void* ptr1 = m61_malloc(1020);
    char* ptr2 = (char*) m61_malloc(2308);
    void* ptr3 = m61_malloc(6161);
    (void) ptr3;
    m61_free(ptr1);
    m61_free(ptr2 + 64);
}

int main() {
    subtest_result r = run_subtest(free_interior_pointer);
    r.print("free of interior pointer");
    assert(r.aborted());

    r = run_subtest(free_misaligned_pointer);
    r.print("free of misaligned pointer");
    assert(r.aborted());

    r = run_subtest(free_pointer_into_freed_allocation);
    r.print("free of pointer into freed allocation");
    assert(r.aborted());

    r = run_subtest(free_interior_pointer_with_active_neighbors);
    r.print("free of interior pointer with active neighbors");
    assert(r.aborted());
}
