#include "heap.h"
#include "pmm.h"

#define FRAME_SIZE 4096

/*
 * Each allocation is preceded by a block_t header.
 * The free list is singly-linked and unsorted
 */
typedef struct block_t {
    uint32_t size;
    uint32_t free;
    struct block_t *next;
} block_t;

struct block_t *heap_list;

void heap_init(void) {
    heap_list = (block_t *)0;
}

void *kmalloc(uint32_t size) {
    if (size == 0)
        return (void *)0;

    size = (size + 3u) & ~3u; // round up to 4 byte alignment
    int got_frame = 0;

    // Search free list
    while (1) {
        block_t *prev = (block_t *)0;
        block_t *cur = heap_list;

        while (cur) {
            if (cur->free && cur->size >= size) {
                // split if leftover is large enough
                if (cur->size >= size + (uint32_t)sizeof(block_t) + 4u) {
                    block_t *split = (block_t *)((uint8_t *)(cur + 1) + size);

                    split->size = cur->size - size - (uint32_t)sizeof(block_t);
                    split->free = 1;
                    split->next = cur->next;

                    cur->size = size;
                    cur->next = split;
                }

                cur->free = 0;
                return (void *)(cur + 1);
            }

            prev = cur;
            cur = cur->next;
        }

        // No suitable block found
        if (got_frame || size > FRAME_SIZE - (uint32_t)sizeof(block_t))
            return (void *)0;

        got_frame = 1;

        uint32_t frame = pmm_alloc_frame();
        if (frame == 0)
            return (void *)0;

        block_t *nb = (block_t *)frame;
        nb->size = FRAME_SIZE - (uint32_t)sizeof(block_t);
        nb->free = 1;
        nb->next = (block_t *)0;

        if (prev)
            prev->next = nb;
        else
            heap_list = nb;
    }
}

void kfree(void *ptr) {
    if (!ptr)
        return;

    block_t *hdr = (block_t *)ptr - 1;
    hdr->free = 1;

    // free blocks
    block_t *cur = heap_list;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += (uint32_t)sizeof(block_t) + cur->next->size;
            cur->next = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}
