#ifndef HEAP_H
#define HEAP_H
#include <stdint.h>
#include <stddef.h>

// The hidden metadata placed before every allocated block
typedef struct heap_block {
    uint32_t size;             // Size of the memory block (excluding this header)
    uint8_t is_free;           // 1 if free, 0 if in use
    struct heap_block* next;   // Pointer to the next block in memory
} __attribute__((packed)) heap_block_t;

void init_heap(void);
void* kmalloc(uint32_t size);
void kfree(void* ptr);

#endif