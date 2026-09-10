#include "m61.hh"

void* m61_malloc(size_t sz) {
    return malloc(sz);
}

void m61_free(void* ptr) {
    free(ptr);
}

m61_statistics m61_get_statistics() {
    return m61_statistics{};
}
