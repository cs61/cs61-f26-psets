#include "m61.hh"
// Check that statistics are initially zero.

int main() {
    m61_statistics stats = m61_get_statistics();
    assert(stats.total_count == 0);
    assert(stats.total_bytes == 0);
    assert(stats.active_count == 0);
    assert(stats.active_bytes == 0);
}
