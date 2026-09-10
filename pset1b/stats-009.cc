#include "m61.hh"
#include "utils.hh"

int main() {
    random_source rand;

    // pick a random number between 1 and 6161, inclusive
    size_t sz1 = rand.uniform(1, 6161);
    void* ptr1 = m61_malloc(sz1);
    assert(ptr1 != nullptr);

    size_t sz2 = rand.uniform(1, 6161);
    void* ptr2 = m61_malloc(sz2);
    assert(ptr2 != nullptr);
    assert(ptr1 != ptr2);

    // flip a coin
    bool del2 = rand.coin_flip();
    if (del2) {
        m61_free(ptr2);
    } else {
        m61_free(ptr1);
    }

    m61_statistics stats = m61_get_statistics();
    
    assert(stats.total_count == 2);
    assert(stats.total_bytes == sz1 + sz2);
    assert(stats.active_count == 1);
    assert(stats.active_bytes == (del2 ? sz1 : sz2));

    printf("Success!\n");
}
