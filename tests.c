#include "virtual_alloc.h"
#include "virtual_sbrk.h"
#include <stdlib.h>
#define HEAP_SIZE_LIMIT 20
#define VIRTUAL_HEAP_SIZE 18
#define MIN_BLOCK_SIZE 11

void * virtual_heap = NULL;

void * virtual_sbrk(int32_t increment) {
    // Your implementation here (for your testing only)
    static int64_t counter = 0;
    void * ret = virtual_heap + counter;
    counter = counter + increment;
    return ret;
}

int main() {
    // Your own testing code here
    /*
     * Before Testing
     */
    virtual_heap = malloc(pow_of_2(HEAP_SIZE_LIMIT) * sizeof(u_int8_t));
    virtual_sbrk(VIRTUAL_HEAP_SIZE);

    init_allocator(virtual_heap, VIRTUAL_HEAP_SIZE, MIN_BLOCK_SIZE);
    virtual_info(virtual_heap);

    /*
     * Test Allocating
     */

//    void * test1 = virtual_malloc(virtual_heap,1024);
//    void * test2 = virtual_malloc(virtual_heap,512);
//    void * test3 = virtual_malloc(virtual_heap,256);
//    void * test4 = virtual_malloc(virtual_heap,256);
//
//    virtual_info(virtual_heap);
//
//    virtual_free(virtual_heap,test4);
//    virtual_free(virtual_heap,test2);
//    virtual_info(virtual_heap);
//    virtual_free(virtual_heap,test1);
//    virtual_free(virtual_heap,test3);

    virtual_info(virtual_heap);
//    printf("\n");
//
//    printf("%d\n",virtual_free(virtual_heap,test2));
//    virtual_info(virtual_heap);
//    void * test5 = virtual_malloc(virtual_heap,256);
//    virtual_info(virtual_heap);
//    printf("%d\n",virtual_free(virtual_heap,test1));
//    virtual_info(virtual_heap);
//    void * test6 = virtual_malloc(virtual_heap,256);
//    virtual_info(virtual_heap);
//    printf("%d\n",virtual_free(virtual_heap,test3));
//    virtual_info(virtual_heap);
//    printf("%d\n",virtual_free(virtual_heap,test5));
//    virtual_info(virtual_heap);
//    printf("%d\n",virtual_free(virtual_heap,test4));
//    virtual_info(virtual_heap);
//    printf("%d\n",virtual_free(virtual_heap,test6));
//    virtual_info(virtual_heap);

    void * test1 = virtual_malloc(virtual_heap,1024);
    void * test2 = virtual_malloc(virtual_heap,1024);
    void * test3 = virtual_malloc(virtual_heap,1024);
    void * test4 = virtual_malloc(virtual_heap,1024);
    void * test5 = virtual_malloc(virtual_heap,1024);
    void * test6 = virtual_malloc(virtual_heap,1024);
    void * test7 = virtual_malloc(virtual_heap,1024);
    void * test8 = virtual_malloc(virtual_heap,1024);
    void * test9 = virtual_malloc(virtual_heap,1024);
    void * test10 = virtual_malloc(virtual_heap,1024);
    void * test11 = virtual_malloc(virtual_heap,1024);
    void * test12 = virtual_malloc(virtual_heap,1024);
    void * test13 = virtual_malloc(virtual_heap,1024);
    void * test14 = virtual_malloc(virtual_heap,1024);
    void * test15 = virtual_malloc(virtual_heap,1024);
    void * test16 = virtual_malloc(virtual_heap,1024);



    printf("%d\n",virtual_free(virtual_heap,test4));
    printf("%d\n",virtual_free(virtual_heap,test14));
    printf("%d\n",virtual_free(virtual_heap,test13));

    printf("%d\n",virtual_free(virtual_heap,test10));
    printf("%d\n",virtual_free(virtual_heap,test8));
    printf("%d\n",virtual_free(virtual_heap,test12));
    printf("%d\n",virtual_free(virtual_heap,test11));
    printf("%d\n",virtual_free(virtual_heap,test9));
    printf("%d\n",virtual_free(virtual_heap,test7));
    printf("%d\n",virtual_free(virtual_heap,test15));
    printf("%d\n",virtual_free(virtual_heap,test2));
    virtual_info(virtual_heap);

    printf("%d\n",virtual_free(virtual_heap,test16));

    printf("%d\n",virtual_free(virtual_heap,test3));
    printf("%d\n",virtual_free(virtual_heap,test1));
    printf("%d\n",virtual_free(virtual_heap,test5));
    printf("%d\n",virtual_free(virtual_heap,test6));



    /*
     * After Testing
     */
    free(virtual_heap);
    virtual_heap = NULL;
    return 0;
}
