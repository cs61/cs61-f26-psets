#include "m61.hh"
#include "utils.hh"
#include <algorithm>
// Check that double frees are detected, including after coalescing.

static void simple_double_free() {
    void* ptr = m61_malloc(2001);
    m61_free(ptr);
    m61_free(ptr);
}

static void double_free_after_coalescing_with_predecessor() {
    void* p = m61_malloc(32);
    void* q = m61_malloc(48);
    m61_free(p);
    m61_free(q);
    m61_free(q);
}

static void double_free_after_coalescing_both_sides() {
    void* ptr1 = m61_malloc(2001);
    void* ptr2 = m61_malloc(100);
    void* ptr3 = m61_malloc(2000);
    m61_free(ptr1);
    m61_free(ptr3);
    m61_free(ptr2);
    m61_free(ptr2);
}

static void double_free_after_other_operations() {
    constexpr int nptrs = 13;
    void* ptrs[nptrs];
    for (int i = 0; i != nptrs; ++i) {
        ptrs[i] = m61_malloc(10);
    }
    std::sort(ptrs, ptrs + nptrs);
    for (int i = 0; i < nptrs; i += 2) {
        m61_free(ptrs[i]);
    }
    (void) m61_malloc(1000);
    (void) m61_malloc(2000);
    m61_free(ptrs[2]);
}

int main() {
    subtest_result r = run_subtest(simple_double_free);
    r.print("simple double free");
    assert(r.aborted());

    r = run_subtest(double_free_after_coalescing_with_predecessor);
    r.print("double free after coalescing with predecessor");
    assert(r.aborted());

    r = run_subtest(double_free_after_coalescing_both_sides);
    r.print("double free after coalescing on both sides");
    assert(r.aborted());

    r = run_subtest(double_free_after_other_operations);
    r.print("double free after other operations");
    assert(r.aborted());
}
