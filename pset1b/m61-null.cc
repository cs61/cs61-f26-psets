#include "m61.hh"

void* m61_malloc(size_t) {
    return nullptr;
}

void m61_free(void*) {
}

m61_statistics m61_get_statistics() {
    return m61_statistics{};
}
