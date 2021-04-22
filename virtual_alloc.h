#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BYTE uint8_t

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size);

void * virtual_malloc(void * heapstart, uint32_t size);

int virtual_free(void * heapstart, void * ptr);

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size);

void virtual_info(void * heapstart);

typedef struct Header {
    uint8_t size;
    uint8_t status;
    uint8_t serial;
    struct Header* next;
} Header;

typedef struct Start {
    uint8_t init_size;
    uint8_t min_size;
    struct Header* first;
} Start;

int merge_and_clear(Header * left, Header * right);

int available_size(void * heapstart, Header * current, Header * next, uint8_t size, uint8_t serial);

uint64_t pow_of_2(uint8_t power);