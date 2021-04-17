#include "virtual_alloc.h"
#include <stdlib.h>

void * virtual_heap = NULL;

void * virtual_sbrk(int32_t increment) {
    // Your implementation here (for your testing only)
    return (void *)(-1);
}

int main() {
    // Your own testing code here
    virtual_heap = malloc(32768 * sizeof(u_int8_t));
    init_allocator(virtual_heap, 15, 12);

    return 0;
}
