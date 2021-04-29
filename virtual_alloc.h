#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BYTE uint8_t
#define HEADER uint8_t
#define START uint8_t
#define HEADER_SIZE 1
#define HEAPSTART_SIZE 2
#define FREE 0
#define IN_USE 1

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size);

void * virtual_malloc(void * heapstart, uint32_t size);

int virtual_free(void * heapstart, void * ptr);

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size);

void virtual_info(void * heapstart);

int available_size(void * heapstart, HEADER * previous, HEADER * next, uint8_t size, uint8_t serial);

uint64_t pow_of_2(uint8_t power);