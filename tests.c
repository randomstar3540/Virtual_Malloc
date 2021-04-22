#include "virtual_alloc.h"
#include <stdlib.h>
#define HEAP_SIZE_LIMIT 20
#define VIRTUAL_HEAP_SIZE 11
#define MIN_BLOCK_SIZE 8

void * virtual_heap = NULL;

void * virtual_sbrk(int32_t increment) {
    // Your implementation here (for your testing only)
    static int64_t counter = 0;
    void * ret = virtual_heap + counter;
    if(pow_of_2(HEAP_SIZE_LIMIT) - counter < increment){
        return NULL;
    }
    counter += increment;
    return ret;
}

void debug(void * heapstart){
    Header * header_ptr = ((Start*)heapstart)->first;
    uint64_t count = 0;
    uint64_t current_size;
    BYTE * current_address = (BYTE *) (((Start *) heapstart) + 1);

    printf("\n\nDEBUG Information\n");
    printf("Heapstart: %p\n",heapstart);
    printf("Program break: %p\n\n",virtual_sbrk(0));


    while (header_ptr != NULL){
        current_size = pow_of_2(header_ptr->size);

        /*
         * Printing
         */
        printf("BLOCK %lu\n",count);
        printf("ADDR: %p\n",current_address);
        printf("size : %d\n",header_ptr->size);
        printf("serial : %d\n",header_ptr->serial);
        if (header_ptr->status){
            printf("status : IN USE %d\n",header_ptr->status);
        }else{
            printf("status : FREE %d\n",header_ptr->status);
        }
        printf("\n");


        header_ptr = header_ptr->next;
        current_address += current_size;
        count ++;
    }
}

int main() {
    // Your own testing code here
    /*
     * Before Testing
     */
    virtual_heap = malloc(pow_of_2(HEAP_SIZE_LIMIT) * sizeof(u_int8_t));
    virtual_sbrk(VIRTUAL_HEAP_SIZE);

    init_allocator(virtual_heap, VIRTUAL_HEAP_SIZE, MIN_BLOCK_SIZE);

    /*
     * Test Allocating
     */

    void * test1 = virtual_malloc(virtual_heap,1024);
    void * test2 = virtual_malloc(virtual_heap,512);
    void * test3 = virtual_malloc(virtual_heap,256);
    void * test4 = virtual_malloc(virtual_heap,256);

    virtual_info(virtual_heap);
    printf("\n");

    printf("%d\n",virtual_free(virtual_heap,test2));
    virtual_info(virtual_heap);
    void * test5 = virtual_malloc(virtual_heap,256);
    virtual_info(virtual_heap);
    printf("%d\n",virtual_free(virtual_heap,test1));
    virtual_info(virtual_heap);
    void * test6 = virtual_malloc(virtual_heap,256);
    virtual_info(virtual_heap);
    printf("%d\n",virtual_free(virtual_heap,test3));
    virtual_info(virtual_heap);
    printf("%d\n",virtual_free(virtual_heap,test5));
    virtual_info(virtual_heap);
    printf("%d\n",virtual_free(virtual_heap,test4));
    virtual_info(virtual_heap);
    printf("%d\n",virtual_free(virtual_heap,test6));
    virtual_info(virtual_heap);



    /*
     * After Testing
     */
    free(virtual_heap);
    virtual_heap = NULL;
    return 0;
}
