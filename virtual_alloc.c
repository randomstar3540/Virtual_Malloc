#include "virtual_alloc.h"

void * virtual_sbrk(int32_t increment);

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {
    // Your code here
    printf("Heap start is %p\n",heapstart);
    printf("Heap end is %p\n",virtual_sbrk(1));
    printf("Heap end is %p\n",virtual_sbrk(1));
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
