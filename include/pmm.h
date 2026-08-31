#ifndef PMM_H
#define PMM_H
#include <stdint.h>

void pmm_init(void);
void pmm_mark_used(uint32_t base_address, uint32_t size);
void pmm_mark_free(uint32_t base_address, uint32_t size);


void* pmm_alloc_frame(void);
void pmm_free_frame(void* physical_address);

#endif