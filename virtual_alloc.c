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

int validation(void * heapstart){
    /*
     * Check if the allocating data structure is valid
     * If some unexpected behavior happened, or data structure modification detected
     * it will report an error
     */
    if(heapstart==NULL){
        return -1;
    }
    //check if initial size and minimum size valid
    if (read_init_size(heapstart) > 64 || read_min_size(heapstart) > 64){
        return -1;
    }
    if (read_init_size(heapstart) < read_min_size(heapstart)){
        return -1;
    }

    //check if virtual_sbrk working
    if (virtual_sbrk(0) == NULL){
        return -1;
    }

    //Compute the address of the header of first block
    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;
    //compute the address of each block in allocating space
    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;
    uint64_t sum_size = 0;

    while (counter < blocks){

        if(read_size(*header_ptr)>64){
            return -1;
        }

        sum_size += pow_of_2(read_size(*header_ptr));
        header_ptr += HEADER_SIZE;
        counter ++;
    }

    //check if block structure valid
    if (pow_of_2(read_init_size(heapstart)) != sum_size){
        return -1;
    }

    return 0;
}

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {

    if(heapstart==NULL){
        return;
    }
    //calculate current space and extend the program break
    uint64_t current_size = virtual_sbrk(0)-heapstart;

    if (virtual_sbrk(pow_of_2(initial_size) - current_size + HEAPSTART_SIZE) == NULL){
        return;
    }

    HEADER * first_header = virtual_sbrk(0);
    //move program break to next byte
    if (virtual_sbrk(HEADER_SIZE) == NULL){
        return;
    }
    //initialize starting structure and the header of first block
    write_start(heapstart,initial_size,min_size);
    *first_header = 0;
    writer_size(first_header,initial_size);
    writer_status(first_header,FREE);

}

void * virtual_malloc(void * heapstart, uint32_t size) {

    if(heapstart==NULL){
        return NULL;
    }

    if (validation(heapstart)==-1){
        //if validation fail
        return NULL;
    }
    //Compute the address of the header of first block
    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;
    //Initialize as NULL, once there exist suitable block, it will point to the header of that block
    HEADER * best_fit = NULL;
    uint64_t best_fit_size = UINT64_MAX;
    uint64_t current_size;
    BYTE * best_fit_address;
    //compute the address of each block in allocating space
    BYTE * current_address = (BYTE *) (heapstart + HEAPSTART_SIZE);

    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;

    if (size == 0){
        return NULL;
    }

    while (counter < blocks){
        //compute the actual size by taking the power
        current_size = pow_of_2(read_size(*header_ptr));

        if (read_status(*header_ptr) == FREE){
            //Only consider blocks that with status of FREE
            if (current_size >= size && current_size < best_fit_size){
                //Update is there exist a better block
                best_fit = header_ptr;
                best_fit_address = current_address;
                best_fit_size = current_size;
            }
        }

        //compute the address of next block in allocating space
        current_address += current_size;
        //compute the address of the header of next block
        header_ptr += HEADER_SIZE;
        //update counter
        counter ++;
    }

    if(best_fit == NULL){
        //if no suitable block found, return NULL
        return NULL;
    }

    HEADER * new_header;
    uint8_t new_size_exp = read_size(*best_fit) -1;
    uint64_t new_size = pow_of_2(new_size_exp);
    //No matter if we can break, change the status of current block
    writer_status(best_fit,IN_USE);

    while (new_size >= size && new_size_exp >= read_min_size(heapstart)){
        //continue breaking if we can break
        new_header = add_block(best_fit);
        if(new_header == NULL){
            //if adding fails
            return NULL;
        }

        //initialize the new block
        writer_status(new_header,FREE);
        writer_size(new_header,new_size_exp);
        //reduce the size of current block
        writer_size(best_fit,new_size_exp);

        //update the variables
        new_size_exp = read_size(*best_fit) - 1;
        new_size = pow_of_2(new_size_exp);
    }

    return best_fit_address;
}

int virtual_free(void * heapstart, void * ptr) {

    if(heapstart==NULL){
        return 1;
    }

    if (validation(heapstart)==-1){
        //if validation fail
        return 1;
    }
    //Compute the address of the header of first block
    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;
    HEADER * previous_ptr = NULL;
    HEADER * next_ptr = NULL;
    uint64_t current_size;

    //previous address needed for recursive case
    BYTE * previous_address = NULL;
    BYTE * current_address = (BYTE *) (heapstart + HEAPSTART_SIZE);

    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;

    while (counter < blocks){
        //update next header
        next_ptr = header_ptr + HEADER_SIZE;
        current_size = pow_of_2(read_size(*header_ptr));

        if (current_address == ptr){
            //if block address matches ptr
            //update status to free no matter if it is going to recursive
            writer_status(header_ptr, FREE);
            uint64_t serial = count_serial(heapstart,header_ptr);

            //break the recursive if it goes to the maximum size
            if(read_size(*header_ptr) >= read_init_size(heapstart)){
                return 0;
            }

            if(previous_ptr != NULL && serial % 2 == 1){
                //if serial is odd, try merge with the left(previous) block
                if (read_size(*previous_ptr) == read_size(*header_ptr) && read_status(*previous_ptr) == FREE){
                    //merge only if both is free and size is same

                    //update size and remove the right side(current) block
                    writer_size(previous_ptr,read_size(*previous_ptr + 1));
                    remove_block(header_ptr);

                    //recursively free
                    virtual_free(heapstart,previous_address);
                    return 0;
                }
            }else if (next_ptr != NULL && serial % 2 == 0){
                //if serial is even, try merge with the right(next) block
                if (read_size(*header_ptr) == read_size(*next_ptr) && read_status(*next_ptr) == FREE){
                    //merge only if both is free and size is same

                    //update size and remove the right side(next) block
                    writer_size(header_ptr,read_size(*header_ptr + 1));
                    remove_block(next_ptr);

                    //recursively free
                    virtual_free(heapstart,current_address);
                    return 0;
                }
            }
            return 0;
        }

        //update looping variables
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

    if (validation(heapstart)==-1){
        //if validation fail
        return NULL;
    }

    //Compute the address of the header of first block
    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;
    //the block header which we reallocate to
    HEADER * realloc_header;
    uint64_t current_size;
    uint64_t max_available_size = 0; //the maximum size we can obtain
    uint64_t size_obtain_free; //the maximum size we can obtain if we free the given block
    BYTE * current_address = (BYTE *) (heapstart + HEAPSTART_SIZE);
    BYTE * new_address;

    if(ptr == NULL){
        //if pointer is NULL, go to malloc
        return virtual_malloc(heapstart,size);
    }

    if(size == 0){
        //if size is 0, go to free
        virtual_free(heapstart,ptr);
        ptr = NULL;
        return NULL;
    }

    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;

    while (counter < blocks){
        current_size = pow_of_2(read_size(*header_ptr));
        if (read_status(*header_ptr) == FREE && current_size>max_available_size){
            //if block is free and maximum can be updated
            max_available_size = current_size;
        }

        if (current_address == ptr){
            realloc_header = header_ptr;
            //compute the size we can obtain if we free the block
            size_obtain_free = available_size(heapstart, header_ptr -1, header_ptr +1,
                                              read_size(*header_ptr),count_serial(heapstart,header_ptr));
            size_obtain_free = pow_of_2(size_obtain_free);

            if(size_obtain_free > max_available_size){
                //update if needed
                max_available_size = size_obtain_free;
            }
        }

        //update looping variables
        current_address += current_size;
        header_ptr += HEADER_SIZE;
        counter ++;
    }

    if (realloc_header != NULL && max_available_size >= size){
        //if the size we can obtain is larger than the size we are going to reallocate
        //Just free current block and allocate it again
        uint64_t original = pow_of_2(read_size(*realloc_header));
        virtual_free(heapstart,ptr);
        new_address = virtual_malloc(heapstart,size);
        //take the smaller one between current size and reallocate size
        size = original > size ? size : original;
        //move the contents from previous to the new block
        memmove(new_address,ptr,size);
        return new_address;
    }

    return NULL;
}

void virtual_info(void * heapstart) {

    if(heapstart==NULL){
        return;
    }

    if (validation(heapstart)==-1){
        //if validation fail
        return;
    }
    //Compute the address of the header of first block
    HEADER * header_ptr = heapstart + pow_of_2(read_init_size(heapstart)) + HEAPSTART_SIZE;

    uint64_t blocks = virtual_sbrk(0) - (void *)header_ptr;
    uint64_t counter = 0;
    while (counter < blocks){
        //continue reading and printing blocks
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
    //preform a false-free operation, the data needed for this false-block is in parameters
    if (size >= read_init_size(heapstart)){
        //break recursion
        return size;
    }

    //calculating serial number and recursively compute just like free
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
    //compute the power of 2 by the exponential given
    uint64_t num = 1;
    return num << power;
}
