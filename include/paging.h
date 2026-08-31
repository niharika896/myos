#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>

//security flags
#define PAGE_PRESENT    1
#define PAGE_WRITE      2  
#define PAGE_USER       4   

typedef struct {
    uint32_t entries[1024];
} page_table_t;

typedef struct {
    uint32_t entries[1024];
} page_directory_t;

void init_paging(void);

void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags); 
#endif