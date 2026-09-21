#include "m61.hh"
#include "utils.hh"
#include <cstring>
// Check that writes just past the end of an allocation are detected when
// the allocation is freed.

static void off_by_one_index() {
    int* array = (int*) m61_malloc(10 * sizeof(int));
    for (int i = 0; i <= 10 /* Whoops! Should be < */; ++i) {
        array[i] = i;
    }
    m61_free(array);
}

static void forgot_terminating_nul() {
    const char* s = "Hello, this is a string! I exist to demonstrate a common error.";
    char* copy = (char*) m61_malloc(strlen(s));
    strcpy(copy, s);
    m61_free(copy);
}

static void forgot_sizeof() {
    int* array = (int*) m61_malloc(3);   // oops, forgot `* sizeof(int)`
    for (int i = 0; i != 3; ++i) {
        array[i] = 0;
    }
    m61_free(array);
}

static void write_past_aligned_block() {
    // the block's size is a multiple of 16
    int* array = (int*) m61_malloc(16 * sizeof(int));
    for (int i = 0; i != 18; ++i) {
        array[i] = 0;
    }
    m61_free(array);
}

int main() {
    subtest_result r = run_subtest(off_by_one_index);
    r.print("off-by-one array index");
    assert(r.aborted());

    r = run_subtest(forgot_terminating_nul);
    r.print("string copy without room for the terminating NUL");
    assert(r.aborted());

    r = run_subtest(forgot_sizeof);
    r.print("forgot sizeof, wrote several zero bytes past the end");
    assert(r.aborted());

    r = run_subtest(write_past_aligned_block);
    r.print("write past the end of a 64-byte block");
    assert(r.aborted());
}
