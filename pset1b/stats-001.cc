// https://edstem.org/us/courses/105347/discussion/8244931
// group: stats

//This tests that the statistics changed in expected ways
# include "m61.hh"
# include <cassert>

int main() {
    m61_statistics stats1 = m61_get_statistics();

    void* ptr1 = m61_malloc(100);
    
    assert(ptr1 != nullptr);
    
    m61_statistics stats2 = m61_get_statistics();
    
    assert(stats2.active_count == stats1.active_count + 1);
    assert(stats2.total_count == stats1.total_count + 1);
    assert(stats2.total_bytes == stats1.total_bytes + 100);

    printf("Success!\n");
}
