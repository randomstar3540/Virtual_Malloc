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

    if (size == 0){
        return NULL;
    }

    while (header_ptr != NULL){
        current_size = pow_of_2(header_ptr->size);

        if (header_ptr->status==0){
            if (current_size >= size && current_size < best_fit_size){
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

    while (pow_of_2(best_fit->size-1) >= size && best_fit->size-1 >= ((Start*)heapstart)->min_size){
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

    return best_fit_address;
}

int virtual_free(void * heapstart, void * ptr) {
    // Your code here
    Header * header_ptr = ((Start*)heapstart)->first;
    Header * previous = NULL;
    Header * next = NULL;

    uint64_t current_size;

    BYTE * previous_address = (BYTE *) (((Start *) heapstart) + 1);
    BYTE * current_address = (BYTE *) (((Start *) heapstart) + 1);

    while (header_ptr != NULL){
        next = header_ptr->next;
        current_size = pow_of_2(header_ptr->size);
        if (current_address == ptr){
            header_ptr->status = 0;

            if(header_ptr->size >= ((Start*)heapstart)->init_size){
                return 0;
            }

            if(next != NULL && header_ptr->serial % 2 == 0){
                if (header_ptr->size == next->size && next->status == 0){
                    merge_and_clear(header_ptr,next);
                    virtual_free(heapstart,current_address);
                }
            }else if (previous != NULL && header_ptr->serial % 2 == 1){
                if (previous->size == header_ptr->size && previous->status == 0){
                    merge_and_clear(previous,header_ptr);
                    virtual_free(heapstart,previous_address);
                }
            }
            return 0;
        }

        previous_address = current_address;
        current_address += current_size;
        previous = header_ptr;
        header_ptr = header_ptr->next;
    }

    return 1;
}

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size) {
    Header * header_ptr = ((Start*)heapstart)->first;
    Header * realloc_header;
    uint64_t current_size;
    uint64_t max_available_size = 0;
    uint64_t size_obtain_free;
    BYTE * current_address = (BYTE *) (((Start *) heapstart) + 1);
    BYTE * new_address;

    if(ptr == NULL){
        return virtual_malloc(heapstart,size);
    }

    if(size == 0){
        virtual_free(heapstart,ptr);
        ptr = NULL;
        return NULL;
    }

    while (header_ptr != NULL){
        current_size = pow_of_2(header_ptr->size);
        if (header_ptr->status == 0 && current_size>max_available_size){
            max_available_size = current_size;
        }

        if (current_address == ptr){
            realloc_header = header_ptr;

            size_obtain_free = available_size(
                    heapstart,header_ptr,header_ptr->next,header_ptr->size,header_ptr->serial);
            size_obtain_free = pow_of_2(size_obtain_free);

            if(size_obtain_free > max_available_size){
                max_available_size = size_obtain_free;
            }
        }

        current_address += current_size;
        header_ptr = header_ptr->next;
    }

    if (realloc_header != NULL && max_available_size >= size){
        uint64_t original = pow_of_2(realloc_header->size);
        virtual_free(heapstart,ptr);
        new_address = virtual_malloc(heapstart,size);
        size = original > size ? size : original;
        memmove(new_address,ptr,size);
        return new_address;
    }

    return NULL;
}

void virtual_info(void * heapstart) {
    Header * header_ptr = ((Start*)heapstart)->first;
    while (header_ptr != NULL){
        if (header_ptr->status == 0){
            printf("free %lu\n",pow_of_2(header_ptr->size));
        } else if (header_ptr->status ==1){
            printf("allocated %lu\n",pow_of_2(header_ptr->size));
        } else {
            return;
        }
        header_ptr = header_ptr->next;
    }
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

int available_size(void * heapstart, Header * current, Header * next, uint8_t size, uint8_t serial){

    if (size >= ((Start*)heapstart)->init_size){
        return size;
    }

    if(serial % 2 == 0){
        if(size == next->size && next->status == 0){
            return available_size(heapstart,current,next->next,size+1, serial/2);
        }else{
            return size;
        }
    }else if (serial % 2 == 1) {
        Header * header_ptr = ((Start*)heapstart)->first;
        Header * previous = NULL;

        while (header_ptr != NULL){
            if(header_ptr == current){
                if(size == previous->size && previous->status == 0){
                    return available_size(heapstart,previous,next,size+1, serial/2);
                }else{
                    return size;
                }
            }
            previous = header_ptr;
            header_ptr = header_ptr->next;
        }
    }

    return -1;

}

uint64_t pow_of_2(uint8_t power){
    uint64_t num = 1;
    return num << power;
}
