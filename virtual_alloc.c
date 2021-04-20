#include "virtual_alloc.h"

void * virtual_sbrk(int32_t increment);

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {
    // Your code here
    if(heapstart==NULL){
        return;
    }

    uint64_t current_size = virtual_sbrk(0)-heapstart;

    if (virtual_sbrk(pow_of_2(initial_size) - current_size + sizeof(Start)) == NULL){
        return;
    }

    Header* first_header = virtual_sbrk(0);

    if (virtual_sbrk(sizeof(Header)) == NULL){
        return;
    }

    ((Start*)heapstart)->init_size = initial_size;
    ((Start*)heapstart)->min_size = min_size;
    ((Start*)heapstart)->first = first_header;

    first_header->size = initial_size;
    first_header->status = 0;
    first_header->serial = 0;
    first_header->next = NULL;

//    Debug
//    printf("%p\n",heapstart);
//    printf("%p\n",first_header->start);
//    printf("%p\n",virtual_sbrk(0));

}

void * virtual_malloc(void * heapstart, uint32_t size) {
    // Your code here
    Header * header_ptr = ((Start*)heapstart)->first;
    Header * best_fit = NULL;
    uint64_t best_fit_size = UINT64_MAX;
    uint64_t current_size;
    BYTE * best_fit_address = (BYTE *) (((Start *) heapstart) + 1);
    BYTE * current_address = (BYTE *) (((Start *) heapstart) + 1);

    while (header_ptr != NULL){
        current_size = pow_of_2(header_ptr->size);

        if (header_ptr->status==0){
            if (current_size > size && current_size < best_fit_size){
                best_fit = header_ptr;
                best_fit_address = current_address;
                best_fit_size = current_size;
            }
        }
        current_address += current_size;
        header_ptr = header_ptr->next;
    }

    if(best_fit == NULL){
        return NULL;
    }

    Header * new_header;
    best_fit->status = 1;

    while (pow_of_2(best_fit->size-1) > size && best_fit->size > ((Start*)heapstart)->min_size){
        new_header = virtual_sbrk(0);
        if (virtual_sbrk(sizeof(Header)) == NULL){
            return NULL;
        }
        new_header->size = best_fit->size-1;
        new_header->status = 0;
        new_header->serial = best_fit->serial*2 + 1;
        new_header->next = best_fit->next;

        best_fit->size = best_fit->size -1;
        best_fit->serial = best_fit->serial*2;
        best_fit->next = new_header;

    }

    uint64_t block_size = pow_of_2(best_fit->size);
    printf("allocated %lu\n", block_size);

    return best_fit_address;
}

int virtual_free(void * heapstart, void * ptr) {
    // Your code here
    Header * current = ((Start*)heapstart)->first;
    Header * previous = NULL;
    Header * next = NULL;

    uint64_t previous_size;
    uint64_t current_size;

    BYTE * previous_address = (BYTE *) (((Start *) heapstart) + 1);
    BYTE * current_address = (BYTE *) (((Start *) heapstart) + 1);

    while (current != NULL){
        next = current->next;
        previous_size = current_size;
        current_size = pow_of_2(current->size);
        if (current_address == ptr){
            current->status = 0;

            if(current->size >= ((Start*)heapstart)->init_size){
                return 0;
            }

            if(next != NULL && current->serial % 2 == 0){
                if (current->size == next->size && next->status == 0){
                    merge_and_clear(current,next);
                    virtual_free(heapstart,current_address);
                    return 0;
                }
            }else if (previous != NULL && current->serial % 2 == 1){
                if (previous->size == current->size && previous->status == 0){
                    merge_and_clear(previous,current);
                    virtual_free(heapstart,previous_address);
                    return 0;
                }
            }
            break;
        }
        current_address += current_size;
        previous = current;
        previous_address += previous_size;
        current = current->next;
    }

    return 1;
}

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size) {
    // Your code here
    return NULL;
}

void virtual_info(void * heapstart) {
    // Your code here
}

int merge_and_clear(Header * left, Header * right){
    left->size = left->size +1;
    left->next = right->next;
    left->serial = left->serial / 2;
    left->status = 0;

    right->size = 0;
    right->serial = 0;
    right->status = 0;
    right->next = NULL;


    memmove(right,right+1,virtual_sbrk(0) - (void *)(right+1));
    int16_t size = sizeof(struct Header);
    virtual_sbrk(-size);
    right = NULL;
    return 0;
}

uint64_t pow_of_2(uint8_t power){
    uint64_t num = 1;
    return num << power;
}
