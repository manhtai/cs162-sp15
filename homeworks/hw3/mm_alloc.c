/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines.
 */

#include "mm_alloc.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

mm_block* first_heap_block = NULL;

/* new_mm_block create new block for memory allocation */
mm_block* new_mm_block(mm_block* prev, size_t s) {
    mm_block* new_block = sbrk(sizeof(mm_block) + s);
    new_block->size = s;
    new_block->free = 0;
    new_block->prev = prev;
    new_block->next = NULL;
    new_block->curr  = (void*) ((address_t) new_block + (address_t) sizeof(mm_block));
    return new_block;
}

void *mm_malloc(size_t size) {
    if (size == 0) return NULL;

    /* First heap block */
    if (first_heap_block == NULL) {
        first_heap_block = sbrk(0);
        return new_mm_block(NULL, size)->curr;
    }

    mm_block* curr_block = first_heap_block;

    while (curr_block){
        if (curr_block->free && size <= curr_block->size) {

            /* Split block if current block's size is too large */
            size_t size_need = sizeof(mm_block) + size;
            if (size_need < curr_block-> size) {
                mm_block* new_block = (mm_block *) ((address_t) curr_block + size + sizeof(mm_block));

                new_block->size = curr_block->size - size - sizeof(mm_block);
                new_block->prev = curr_block;
                new_block->next = curr_block->next;
                new_block->free = 1;
                new_block->curr = (void*) ((address_t) new_block + (address_t) sizeof(mm_block));

                curr_block->next = new_block;
            }

            curr_block->free = 0;
            curr_block->size = size;

            return curr_block->curr;
        }

        if (curr_block->next == NULL)
            break;

        curr_block = curr_block->next;
    };

    /* If old blocks isn't suffice, create new one */
    curr_block->next = new_mm_block(curr_block->next, size);
    return curr_block->next->curr;
}

void *mm_realloc(void *ptr, size_t size) {
    mm_block* old_block = (mm_block*) ((address_t) ptr - sizeof(mm_block));
    old_block->free = 1;

    mm_block* new_block = mm_malloc(size);
    memmove(new_block, ptr, size);

    return new_block->curr;
}

void mm_free(void *ptr) {
    mm_block* curr_block = first_heap_block;

    while (curr_block) {
        if (curr_block->curr == ptr) {
            curr_block->free = 1;

            /* coalesce blocks */
            if (curr_block->prev != NULL && curr_block->prev->free) {
                curr_block->prev->next = curr_block->next;
                curr_block->prev->size = curr_block->prev->size + sizeof(mm_block) + curr_block->size;
            }
            if (curr_block->next != NULL && curr_block->next->free) {
                curr_block->next = curr_block->next->next;
                curr_block->size = curr_block->size + sizeof(mm_block) + curr_block->next->size;
            }
            break;
        }

        if (curr_block->next == NULL)
            break;

        curr_block = curr_block->next;
    }
}
