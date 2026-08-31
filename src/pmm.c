#include "pmm.h"

#define BLOCK_SIZE 4096
#define MAX_BLOCKS 1048576 
#define BITMAP_SIZE (MAX_BLOCKS / 32)

static uint32_t memory_bitmap[BITMAP_SIZE];

#define SET_BIT(bit)   (memory_bitmap[(bit) / 32] |=  (1 << ((bit) % 32)))
#define CLEAR_BIT(bit) (memory_bitmap[(bit) / 32] &= ~(1 << ((bit) % 32)))

void pmm_init(void) {
    //0xFFFFFFFF means all 32 bits are '1' (Used/Reserved)
    for (int i = 0; i < BITMAP_SIZE; i++) {
        memory_bitmap[i] = 0xFFFFFFFF; 
    }
}

void pmm_mark_used(uint32_t base_address, uint32_t size) {
    uint32_t align = base_address / BLOCK_SIZE;
    uint32_t frames = size / BLOCK_SIZE;
    for (uint32_t i = 0; i < frames; i++) {
        SET_BIT(align + i);
    }
}

void pmm_mark_free(uint32_t base_address, uint32_t size) {
    uint32_t align = base_address / BLOCK_SIZE;
    uint32_t frames = size / BLOCK_SIZE;
    for (uint32_t i = 0; i < frames; i++) {
        CLEAR_BIT(align + i);
    }
}

static int pmm_find_free(void){
    for(uint32_t i=0;i<BITMAP_SIZE;i++){
        if(memory_bitmap[i]!=0xFFFFFFFF){
            for(int j=0;j<32;j++){
                if(!(memory_bitmap[i] & (1<<j))){
                    return (i*32)+j;
                }
            }
        }
    }
    return -1;
}

void* pmm_alloc_frame(void){
    int frame = pmm_find_free();
    if(frame==-1){
        return 0;
    }

    SET_BIT(frame);

    uint32_t physical_address = frame * BLOCK_SIZE;

    return (void*)physical_address;
}

void pmm_free_frame(void* physical_address){
    uint32_t addr = (uint32_t)physical_address;
    uint32_t frame = addr/BLOCK_SIZE;
    CLEAR_BIT(frame);
}