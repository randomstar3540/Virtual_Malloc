#include "virtual_alloc.h"
extern void * virtual_sbrk;


void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {
    // Your code here
    printf("Heap start is %p",heapstart);
    printf("Heap end is %p",virtual_sbrk(0));
}

void * virtual_malloc(void * heapstart, uint32_t size) {
    // Your code here
    return NULL;
}

int virtual_free(void * heapstart, void * ptr) {
    // Your code here
    return 1;
}

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size) {
    // Your code here
    return NULL;
}

void virtual_info(void * heapstart) {
    // Your code here
}
