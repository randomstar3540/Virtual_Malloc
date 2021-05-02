#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>

#include "virtual_alloc.h"
#include "virtual_sbrk.h"
#include "cmocka.h"

#define HEAP_SIZE_LIMIT 20
#define LARGE_HEAP_SIZE 18
#define NORMAL_HEAP_SIZE 16
#define SMALL_HEAP_SIZE 11

#define LARGE_BLOCK_SIZE 12
#define NORMAL_BLOCK_SIZE 10
#define SMALL_BLOCK_SIZE 8

void * virtual_heap = NULL;

void * virtual_sbrk(int32_t increment) {
    // Your implementation here (for your testing only)
    static int64_t counter = 0;
    void * ret = virtual_heap + counter;
    counter = counter + increment;
    return ret;
}

static int setup_virtual_heap(void **state){
    virtual_heap = malloc(pow_of_2(HEAP_SIZE_LIMIT) * sizeof(uint8_t));
    virtual_sbrk(10);
    return 0;
}

static int erase_virtual_heap(void **state){
    free(virtual_heap);
    virtual_heap = NULL;
    return 0;
}

static int compare_heap_info(char * filename){

    //open files
    FILE * expected = fopen(filename,"r");
    FILE * output = fopen("test/out","r");
    if (expected == NULL || output == NULL){
        fclose(expected);
        fclose(output);
        return -1;
    }

    //compare size
    fseek(expected, 0L, SEEK_END);
    int64_t expected_size = ftell(expected);
    fseek(expected, 0L, SEEK_SET);

    fseek(output, 0L, SEEK_END);
    int64_t output_size = ftell(output);
    fseek(output, 0L, SEEK_SET);

    if(expected_size != output_size){
        fclose(expected);
        fclose(output);
        return -1;
    }

    int e;//expected
    int o;//output
    while ((e = getc(expected)) != EOF && (o = getc(output)) != EOF){
        if(e != o){
            fclose(expected);
            fclose(output);
            return -1;
        }
    }
    fclose(expected);
    fclose(output);
    return 0;
}

static void test_virtual_init_1(void **state) {
    init_allocator(virtual_heap, NORMAL_HEAP_SIZE, NORMAL_BLOCK_SIZE);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_init_1") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_init_2(void **state) {
    init_allocator(virtual_heap, LARGE_HEAP_SIZE, LARGE_BLOCK_SIZE);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_init_2") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_init_3(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, SMALL_BLOCK_SIZE);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_init_3") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_malloc_1(void **state) {
    init_allocator(virtual_heap, NORMAL_HEAP_SIZE, NORMAL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,1022);
    void * block2 = virtual_malloc(virtual_heap,1024);
    void * block3 = virtual_malloc(virtual_heap,1025);

    assert_int_equal(block2-block1,1024);
    assert_int_equal(block3-block2,1024);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_malloc_1") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_malloc_2(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, NORMAL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,1022);
    void * block2 = virtual_malloc(virtual_heap,1024);
    void * block3 = virtual_malloc(virtual_heap,1025);

    assert_int_equal(block2-block1,1024);
    assert_null(block3);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_malloc_2") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_malloc_3(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, SMALL_HEAP_SIZE);
    void * block1 = virtual_malloc(virtual_heap,2048);
    void * block2 = virtual_malloc(virtual_heap,1024);
    void * block3 = virtual_malloc(virtual_heap,1025);
    assert_non_null(block1);
    assert_null(block2);
    assert_null(block3);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_malloc_3") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_free_1(void **state) {
    init_allocator(virtual_heap, NORMAL_HEAP_SIZE, NORMAL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,1022);
    void * block2 = virtual_malloc(virtual_heap,1024);
    void * block3 = virtual_malloc(virtual_heap,1025);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    virtual_free(virtual_heap,block2);
    virtual_info(virtual_heap);
    virtual_free(virtual_heap,block1);
    virtual_info(virtual_heap);
    virtual_free(virtual_heap,block3);
    virtual_info(virtual_heap);

    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_free_1") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_free_2(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, NORMAL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,1022);
    void * block2 = virtual_malloc(virtual_heap,1024);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    virtual_free(virtual_heap,block2);
    virtual_info(virtual_heap);
    virtual_free(virtual_heap,block1);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_free_2") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_free_3(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, SMALL_HEAP_SIZE);
    void * block1 = virtual_malloc(virtual_heap,2048);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    virtual_free(virtual_heap,block1);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_free_3") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_realloc_1(void **state) {
    init_allocator(virtual_heap, NORMAL_HEAP_SIZE, NORMAL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,1022);
    void * block2 = virtual_malloc(virtual_heap,1024);
    void * block3 = virtual_malloc(virtual_heap,1025);


    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    void * block4 = virtual_realloc(virtual_heap,block1,1023);
    virtual_info(virtual_heap);
    void * block5 = virtual_realloc(virtual_heap,block4,1024);
    virtual_info(virtual_heap);
    void * block6 = virtual_realloc(virtual_heap,block5,1025);
    virtual_info(virtual_heap);
    void * block7 = virtual_realloc(virtual_heap,block2,1023);
    virtual_info(virtual_heap);
    virtual_free(virtual_heap,block6);
    virtual_free(virtual_heap,block7);
    virtual_free(virtual_heap,block3);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_virtual_realloc_1") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_realloc_2(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, NORMAL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,1022);
    void * block2 = virtual_malloc(virtual_heap,1024);
    void * block3 = virtual_malloc(virtual_heap,1024);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);

    void * block4 = virtual_realloc(virtual_heap,block1,1024);
    virtual_info(virtual_heap);

    void * block5 = virtual_realloc(virtual_heap,block4,1025);
    virtual_info(virtual_heap);

    void * block6 = virtual_realloc(virtual_heap,block4,0);
    virtual_info(virtual_heap);

    void * block7 = virtual_realloc(virtual_heap,block2,2048);
    virtual_info(virtual_heap);

    void * block8 = virtual_realloc(virtual_heap,block7,0);
    virtual_info(virtual_heap);

    void * block9 = virtual_realloc(virtual_heap,block3,2048);
    virtual_info(virtual_heap);

    freopen("/dev/tty","w",stdout);
    assert_non_null(block4);
    assert_null(block5);
    assert_null(block6);
    assert_non_null(block7);
    assert_null(block8);
    assert_non_null(block9);

    if (compare_heap_info("test/test_virtual_realloc_2") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_virtual_realloc_3(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, SMALL_HEAP_SIZE);
    void * block1 = virtual_malloc(virtual_heap,2048);
    void * block2 = virtual_malloc(virtual_heap,1024);
    void * block3 = virtual_malloc(virtual_heap,1025);
    assert_non_null(block1);
    assert_null(block2);
    assert_null(block3);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);

    void * block4 = virtual_realloc(virtual_heap,block1,1024);
    virtual_info(virtual_heap);

    void * block5 = virtual_realloc(virtual_heap,NULL,1024);
    virtual_info(virtual_heap);

    void * block6 = virtual_realloc(virtual_heap,block5,0);
    virtual_info(virtual_heap);

    void * block7 = virtual_realloc(virtual_heap,block4,0);
    virtual_info(virtual_heap);

    freopen("/dev/tty","w",stdout);
    assert_non_null(block4);
    assert_null(block5);
    assert_null(block6);
    assert_null(block7);

    if (compare_heap_info("test/test_virtual_realloc_3") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_error_1(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, SMALL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,2048);

    /*
     * Dirty heapstart pointer
     */
    void * block2 = virtual_malloc(block1,1024);
    assert_non_null(block1);
    assert_null(block2);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_error_1") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_error_2(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, SMALL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,1024);
    void * block2 = virtual_malloc(virtual_heap,1024);
    void * block3 = virtual_malloc(virtual_heap,1025);
    assert_non_null(block1);
    assert_non_null(block2);
    assert_null(block3);

    /*
     * Boundary writing test
     */
    memset(block1,1,1024*sizeof(uint8_t));
    memset(block2,2,1024*sizeof(uint8_t));
    uint8_t * block1_ptr = block1;
    uint8_t * block2_ptr = block2;
    assert_int_equal(*block1_ptr,1);
    assert_int_equal(*(block1_ptr+1),1);
    assert_int_equal(*(block1_ptr+1023),1);
    assert_int_equal(*(block1_ptr+1024),2);
    assert_int_equal(*(block2_ptr-1),1);
    assert_int_equal(*block2_ptr,2);
    assert_int_equal(*(block2_ptr+1023),2);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    if (compare_heap_info("test/test_error_2") != 0){
        fail_msg("heap structure not matched!");
    }
}

static void test_error_3(void **state) {
    init_allocator(virtual_heap, SMALL_HEAP_SIZE, SMALL_BLOCK_SIZE);
    void * block1 = virtual_malloc(virtual_heap,512);
    void * block2 = virtual_malloc(virtual_heap,512);
    void * block3 = virtual_malloc(virtual_heap,512);
    assert_non_null(block1);
    assert_non_null(block2);
    assert_non_null(block3);

    //use temporary file to store the output
    freopen("test/out","w",stdout);
    virtual_info(virtual_heap);
    freopen("/dev/tty","w",stdout);

    /*
     * Change allocating data structure
     */
    memset(block3,20,1026*sizeof(uint8_t));
    void * block4 = virtual_malloc(virtual_heap,512);
    assert_null(block4);
    int rc = virtual_free(virtual_heap,block3);
    assert_int_equal(rc,1);
    void * block5 = virtual_realloc(virtual_heap,block1,513);
    assert_null(block5);

    if (compare_heap_info("test/test_error_3") != 0){
        fail_msg("heap structure not matched!");
    }
}
int main() {
    /*
     * Constructing Unit Test
     */
    const struct CMUnitTest tests[] = {
            cmocka_unit_test_setup_teardown(test_virtual_init_1,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_init_2,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_init_3,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_malloc_1,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_malloc_2,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_malloc_3,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_free_1,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_free_2,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_free_3,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_realloc_1,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_realloc_2,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_virtual_realloc_3,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_error_1,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_error_2,setup_virtual_heap,erase_virtual_heap),
            cmocka_unit_test_setup_teardown(test_error_3,setup_virtual_heap,erase_virtual_heap),
    };

    /*
     * Run tests
     */
    cmocka_run_group_tests(tests, NULL, NULL);

    return 0;
}
