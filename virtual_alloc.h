#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define BYTE uint8_t

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size);

void * virtual_malloc(void * heapstart, uint32_t size);

int virtual_free(void * heapstart, void * ptr);

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size);

void virtual_info(void * heapstart);

uint64_t pow_of_2(uint8_t power);

typedef struct Header {
    uint8_t size;
    uint8_t status;
    void * start;
    struct Header* next;
} Header;

typedef struct Start {
    uint8_t init_size;
    uint8_t min_size;
    struct Header* first;
} Start;