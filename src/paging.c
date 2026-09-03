#include "paging.h"
#include "pmm.h"
#include <stdint.h>

page_directory_t* kernel_directory;

void init_paging(void){
    kernel_directory = (page_directory_t*)pmm_alloc_frame();

    //kernel_directory is now a 4Kb frame

    for(int i=0;i<1024;i++){
        kernel_directory->entries[i] = PAGE_WRITE;
    }

    //4Kb for first page table
    page_table_t* first_table = (page_table_t*)pmm_alloc_frame();

    //each page table holds 1024 entries, each entry contolls a 4Kb chunk so each page table contolls 4MB chunk of memory
    //4KB = 4096 bytes
    for(int i=0;i<1024;i++){
        uint32_t physical_address = i*4096;
        first_table->entries[i] = physical_address|PAGE_PRESENT|PAGE_WRITE|PAGE_USER;
    }
    kernel_directory->entries[0] = (uint32_t)first_table|PAGE_PRESENT|PAGE_WRITE|PAGE_USER;
    //turning on virtual memory
    __asm__ volatile("mov %0, %%cr3"::"r"((uint32_t)kernel_directory));

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    cr0|=0x80000000;
    __asm__ volatile("mov %0, %%cr0"::"r"(cr0));
}

void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {

    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x03FF;

    if (!(kernel_directory->entries[pd_index] & PAGE_PRESENT)) {
        
        uint32_t new_table_phys = (uint32_t) pmm_alloc_frame();
        page_table_t* new_table = (page_table_t*) new_table_phys;
        for (int i = 0; i < 1024; i++) {
            new_table->entries[i] = 0; 
        }
        kernel_directory->entries[pd_index] = new_table_phys | PAGE_PRESENT | PAGE_WRITE | flags;
    }

    uint32_t table_phys = kernel_directory->entries[pd_index] & 0xFFFFF000;
    page_table_t* pt = (page_table_t*) table_phys;

    pt->entries[pt_index] = (physical_addr & 0xFFFFF000) | PAGE_PRESENT | flags;

    __asm__ volatile("invlpg (%0)" ::"r"(virtual_addr) : "memory");
}
//temporary fix to be modified later
void vmm_set_user_page(uint32_t virtual_addr) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x03FF;
    
    kernel_directory->entries[pd_index] |= PAGE_USER;
    
    uint32_t table_phys = kernel_directory->entries[pd_index] & 0xFFFFF000;
    page_table_t* pt = (page_table_t*) table_phys;
    
    pt->entries[pt_index] |= PAGE_USER;
    
    __asm__ volatile("invlpg (%0)" ::"r"(virtual_addr) : "memory");
}