#include "m61.hh"

int main() {
    void* ptr1 = m61_malloc(100);
    void* ptr2 = m61_malloc(10);
    void* ptr3 = m61_malloc(100000);
    void* ptr4 = m61_malloc(1000);

    assert(ptr1 && ptr2 && ptr3 && ptr4);

    m61_statistics stats = m61_get_statistics();

    assert(stats.total_count == 4);
    assert(stats.total_bytes == 101110);
    assert(stats.active_count == 4);

    m61_free(ptr1);
    m61_free(ptr2);
    m61_free(ptr3);
    m61_free(ptr4);

    stats = m61_get_statistics();

    assert(stats.total_count == 4);
    assert(stats.total_bytes == 101110);
    assert(stats.active_count == 0);
    // *** What is `stats.active_bytes` here?

    // Can we reuse memory?

    ptr1 = m61_malloc(255 << 15); // approximately 7.97 * (2**20)

    assert(ptr1);

    stats = m61_get_statistics();

    assert(stats.total_count == 5);
    assert(stats.total_bytes == 101110 + (255 << 15));
    assert(stats.active_count == 1);

    printf("Success!\n");
}
