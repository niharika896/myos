#include "heap.h"
#include "pmm.h"
#include "paging.h"

#define KERNEL_HEAP_START 0xD0000000
#define KERNEL_HEAP_SIZE  (1024 * 1024) // 1 MB of starting heap

static heap_block_t* head = NULL;

void init_heap(void) {
    uint32_t frames = KERNEL_HEAP_SIZE / 4096;
    for (uint32_t i = 0; i < frames; i++) {
        uint32_t phys = (uint32_t)pmm_alloc_frame();
        uint32_t virt = KERNEL_HEAP_START + (i * 4096);
        vmm_map_page(virt, phys, PAGE_WRITE);
    }

    head = (heap_block_t*) KERNEL_HEAP_START;
    head->size = KERNEL_HEAP_SIZE - sizeof(heap_block_t);
    head->is_free = 1;
    head->next = NULL;
}

void* kmalloc(uint32_t size) {
    heap_block_t* current = head;

    // Scan the linked list for the first free block that is large enough
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            
            // If the block is huge, split it to save space
            if (current->size > size + sizeof(heap_block_t) + 4) {
                // Calculate where the new split block will begin
                heap_block_t* new_block = (heap_block_t*)((uint8_t*)current + sizeof(heap_block_t) + size);
                
                new_block->size = current->size - size - sizeof(heap_block_t);
                new_block->is_free = 1;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            current->is_free = 0; // Mark as used
            
            // Return the memory address immediately AFTER the hidden header
            return (void*)((uint8_t*)current + sizeof(heap_block_t));
        }
        current = current->next;
    }
    return NULL; // Out of memory!
}

void kfree(void* ptr) {
    if (!ptr) return;

    // Jump backward in memory to find the hidden header
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    block->is_free = 1;

    // todo: merge adjacent free blocks to prevent fragmentation
}