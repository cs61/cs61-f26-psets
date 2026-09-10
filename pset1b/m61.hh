#ifndef M61_HH
#define M61_HH 1
#include <cstdlib>
#include <cassert>
#include <cstdio>

struct m61_statistics {
    unsigned long long total_count;   // cumulative number of allocations
    unsigned long long total_bytes;   // cumulative number of bytes allocated
    unsigned long long active_count;  // number of active allocations
    unsigned long long active_bytes;  // number of bytes currently allocated
};

void* m61_malloc(size_t sz);
void m61_free(void* ptr);
m61_statistics m61_get_statistics();

#endif
