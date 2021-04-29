#include "virtual_alloc.h"
#include "virtual_sbrk.h"
/*
 * Buddy Data Structure: HEADER
 * Size of HEADER: 1 byte
 *
 * Bits:  0   1   2   3   4   5   6   7
 *      | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *   |Status|Padding|       Size        |
 *
 * Status: FREE 0
 *         IN USE 1
 *
 * Size: 2^(size)
 *
 */

uint8_t read_status(HEADER h){
    // read the status code of a block store in the buddy data structure
    return h >> 7;
}

uint8_t read_size(HEADER h){
    // read the size of a block in the buddy data structure
    h = h << 1;
    return h >> 1;
}

/*
 * Virtual Heap Structure
 * Byte offset:
 * |    0    |    1    | ... 2^(init_size)... | 2^(init_size) + 2| ..... |
 * |init size| min size|   Allocating space   | Allocator Data Structure |
 * |                                                                     |
 * heapstart                                             virtual program break
 *
 */

uint8_t read_init_size(START *s){
    // read the initial size of the virtual heap in heap start
    return (*s);
}

uint8_t read_min_size(START *s){
    // read the minimum size of the virtual heap in heap start
    return (*(s+1));
}

void writer_status(HEADER *h, uint8_t status){
    // Update the status of a block's buddy data structure
    uint8_t size = read_size(*h);
    (*h) = (status << 7) | size;
}

void writer_size(HEADER *h, uint8_t size){
    // Update the size of a block's buddy data structure
    uint8_t status = read_status(*h);
    (*h) = (status << 7) | size;
}

void write_start(START *s, uint8_t init_size, uint8_t min_size){
    //Update the data stores in the heap start
    (*s) = init_size;
    (*(s+1)) = min_size;
}

HEADER * add_block(HEADER *h){
    /*
     * Add a new block after the given block
     * With pushing all blocks behind with 1 byte
     * And moving program break behind for 1 byte
     * Before:
     *  .... | Header h | Next | Next Next | .... |
     *                                      program break
     * After:
     *  .... | Header h | New | Next | Next Next | .... |
     *                                        new program break
     */
    if (virtual_sbrk(HEADER_SIZE) == NULL){
        return NULL;
    }
    HEADER * dest = h + 2 * HEADER_SIZE;
    HEADER * new_block = h + HEADER_SIZE;
    uint64_t size = virtual_sbrk(0) - (void *)new_block;
    memmove(dest,new_block,size);
    *new_block = 0;
    return new_block;
}

HEADER * remove_block(HEADER *h){
    /*
     * Remove the block given
     * With pushing all blocks forward with 1 byte
     * And moving program break forward for 1 byte
     * Before:
     *  .... | Header h | Next | Next Next | .... |
     *                                      program break
     * After:
     *  .... | Next | Next Next | .... |
     *                           new program break
     */
    HEADER * src = h + HEADER_SIZE;
    uint64_t size = virtual_sbrk(0) - (void *)h;
    memmove(h,src,size);

    if (virtual_sbrk(-1) == NULL){
        return NULL;
    }
    return h;
}

int64_t count_serial(void * heapstart, HEADER * h){
    /*
     * Counting the serial in a block
     * By the sum of size of all blocks behind it / its size
     * Use to determining where is its buddy
     * merging left ? or merging right ?
     * Serial examples:
     *      0        1         1
     * |size a-1|size a-1|   size a   |
     *
     *         0         2        3
     * |   size a   |size a-1|size a-1|
     */
    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;
    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;
    uint64_t serial = 0;
    while (counter < blocks){
        if (header_ptr == h){
            //If we get to the block needed, return its serial
            serial = serial / pow_of_2(read_size(*h));
            return serial;
        }
        serial += pow_of_2(read_size(*header_ptr));
        header_ptr += HEADER_SIZE;
        counter ++;
    }
    return -1;
}

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {

    if(heapstart==NULL){
        return;
    }

    uint64_t current_size = virtual_sbrk(0)-heapstart;

    if (virtual_sbrk(pow_of_2(initial_size) - current_size + HEAPSTART_SIZE) == NULL){
        return;
    }

    HEADER * first_header = virtual_sbrk(0);

    if (virtual_sbrk(HEADER_SIZE) == NULL){
        return;
    }

    write_start(heapstart,initial_size,min_size);
    *first_header = 0;
    writer_size(first_header,initial_size);
    writer_status(first_header,FREE);

}

void * virtual_malloc(void * heapstart, uint32_t size) {

    if(heapstart==NULL){
        return NULL;
    }

    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;
    HEADER * best_fit = NULL;
    uint64_t best_fit_size = UINT64_MAX;
    uint64_t current_size;
    BYTE * best_fit_address;
    BYTE * current_address = (BYTE *) (heapstart + HEAPSTART_SIZE);

    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;

    if (size == 0){
        return NULL;
    }

    while (counter < blocks){
        current_size = pow_of_2(read_size(*header_ptr));

        if (read_status(*header_ptr) == FREE){
            if (current_size >= size && current_size < best_fit_size){
                best_fit = header_ptr;
                best_fit_address = current_address;
                best_fit_size = current_size;
            }
        }
        current_address += current_size;
        header_ptr += HEADER_SIZE;
        counter ++;
    }

    if(best_fit == NULL){
        return NULL;
    }

    HEADER * new_header;
    uint8_t new_size_exp = read_size(*best_fit) -1;
    uint64_t new_size = pow_of_2(new_size_exp);
    writer_status(best_fit,IN_USE);

    while (new_size >= size && new_size_exp >= read_min_size(heapstart)){
        new_header = add_block(best_fit);
        if(new_header == NULL){
            return NULL;
        }

        writer_status(new_header,FREE);
        writer_size(new_header,new_size_exp);
        writer_size(best_fit,new_size_exp);

        new_size_exp = read_size(*best_fit) - 1;
        new_size = pow_of_2(new_size_exp);
    }

    return best_fit_address;
}

int virtual_free(void * heapstart, void * ptr) {

    if(heapstart==NULL){
        return 1;
    }

    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;
    HEADER * previous_ptr = NULL;
    HEADER * next_ptr = NULL;
    uint64_t current_size;

    BYTE * previous_address = NULL;
    BYTE * current_address = (BYTE *) (heapstart + HEAPSTART_SIZE);

    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;

    while (counter < blocks){
        next_ptr = header_ptr + HEADER_SIZE;
        current_size = pow_of_2(read_size(*header_ptr));
        if (current_address == ptr){
            writer_status(header_ptr, FREE);
            uint64_t serial = count_serial(heapstart,header_ptr);

            if(read_size(*header_ptr) >= read_init_size(heapstart)){
                return 0;
            }

            if(previous_ptr != NULL && serial % 2 == 1){
                if (read_size(*previous_ptr) == read_size(*header_ptr) && read_status(*previous_ptr) == FREE){
                    writer_size(previous_ptr,read_size(*previous_ptr + 1));
                    remove_block(header_ptr);
                    virtual_free(heapstart,previous_address);
                    return 0;
                }
            }else if (next_ptr != NULL && serial % 2 == 0){
                if (read_size(*header_ptr) == read_size(*next_ptr) && read_status(*next_ptr) == FREE){
                    writer_size(header_ptr,read_size(*header_ptr + 1));
                    remove_block(next_ptr);
                    virtual_free(heapstart,current_address);
                    return 0;
                }
            }
            return 0;
        }

        previous_address = current_address;
        current_address += current_size;
        previous_ptr = header_ptr;
        header_ptr += HEADER_SIZE;
        counter ++;
    }

    return 1;
}

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size) {

    if(heapstart==NULL){
        return NULL;
    }

    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;
    HEADER * realloc_header;
    uint64_t current_size;
    uint64_t max_available_size = 0;
    uint64_t size_obtain_free;
    BYTE * current_address = (BYTE *) (heapstart + HEAPSTART_SIZE);
    BYTE * new_address;

    if(ptr == NULL){
        return virtual_malloc(heapstart,size);
    }

    if(size == 0){
        virtual_free(heapstart,ptr);
        ptr = NULL;
        return NULL;
    }

    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;

    while (counter < blocks){
        current_size = pow_of_2(read_size(*header_ptr));
        if (read_status(*header_ptr) == FREE && current_size>max_available_size){
            max_available_size = current_size;
        }

        if (current_address == ptr){
            realloc_header = header_ptr;

            size_obtain_free = available_size(heapstart, header_ptr -1, header_ptr +1,
                                              read_size(*header_ptr),count_serial(heapstart,header_ptr));
            size_obtain_free = pow_of_2(size_obtain_free);

            if(size_obtain_free > max_available_size){
                max_available_size = size_obtain_free;
            }
        }

        current_address += current_size;
        header_ptr += HEADER_SIZE;
        counter ++;
    }

    if (realloc_header != NULL && max_available_size >= size){
        uint64_t original = pow_of_2(read_size(*realloc_header));
        virtual_free(heapstart,ptr);
        new_address = virtual_malloc(heapstart,size);
        size = original > size ? size : original;
        memmove(new_address,ptr,size);
        return new_address;
    }

    return NULL;
}

void virtual_info(void * heapstart) {

    if(heapstart==NULL){
        return;
    }

    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;

//    printf("HEAP SIZE %lu\n",(void *)header_ptr - heapstart);//debug
//    printf("BRK %p\n",virtual_sbrk(0));//debug

    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;
    while (counter < blocks){
//        printf("BLOCK %lu ADDR %p\n",counter,header_ptr); //debug
        if (read_status(*header_ptr) == FREE){
            printf("free %lu\n",pow_of_2(read_size(*header_ptr)));
        } else if (read_status(*header_ptr) == IN_USE){
            printf("allocated %lu\n",pow_of_2(read_size(*header_ptr)));
        } else {
            return;
        }
        header_ptr += HEADER_SIZE;
        counter ++;
    }
}

int available_size(void * heapstart, HEADER * previous, HEADER * next, uint8_t size, uint8_t serial){

    if (size >= read_init_size(heapstart)){
        return size;
    }

    if(serial % 2 == 0){
        if(size == read_size(*next) && read_status(*next) == FREE){
            return available_size(heapstart,previous,next+1,size+1, serial/2);
        }else{
            return size;
        }
    }else if (serial % 2 == 1) {
        if(size == read_size(*previous) && read_status(*previous) == FREE){
            return available_size(heapstart,previous-1,next,size+1, serial/2);
        }else{
            return size;
        }
    }

    return -1;

}

uint64_t pow_of_2(uint8_t power){
    uint64_t num = 1;
    return num << power;
}
