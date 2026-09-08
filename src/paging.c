#include "paging.h"
#include "pmm.h"
#include <stdint.h>
#include "helpers.h"
#include "task.h"

page_directory_t* kernel_directory;
static uint8_t cow_temp_buffer[4096];


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
//outdated
void vmm_set_user_page(uint32_t virtual_addr) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x03FF;
    
    kernel_directory->entries[pd_index] |= PAGE_USER;
    
    uint32_t table_phys = kernel_directory->entries[pd_index] & 0xFFFFF000;
    page_table_t* pt = (page_table_t*) table_phys;
    
    pt->entries[pt_index] |= PAGE_USER;
    
    __asm__ volatile("invlpg (%0)" ::"r"(virtual_addr) : "memory");
}

page_directory_t* clone_page_directory(){
    page_directory_t* new_pd = (page_directory_t*)pmm_alloc_frame();

    memset(new_pd,0,4096);

   for(int i = 0; i < 1024; i++){
        new_pd->entries[i] = kernel_directory->entries[i];
    }
    return new_pd;
}

void clone_user_pages_cow(page_directory_t* parent_pd, page_directory_t* child_pd){
    for(int pd_idx = 1;pd_idx<768;pd_idx++){
        if(parent_pd->entries[pd_idx] & PAGE_PRESENT){
            uint32_t parent_pt_phys = parent_pd->entries[pd_idx] & 0xFFFFF000;
            page_table_t* parent_pt = (page_table_t*)parent_pt_phys;

            uint32_t child_pt_phys = (uint32_t)pmm_alloc_frame();
            page_table_t* child_pt = (page_table_t*)child_pt_phys;

            memset((void*)child_pt,0,4096);

            child_pd->entries[pd_idx]=child_pt_phys|PAGE_PRESENT|PAGE_WRITE|PAGE_USER;

            for(int pt_idx = 0;pt_idx<1024;pt_idx++){
                if(parent_pt->entries[pt_idx] & PAGE_PRESENT){
                    child_pt->entries[pt_idx] = parent_pt->entries[pt_idx];
                    parent_pt->entries[pt_idx] &= ~PAGE_WRITE;
                    child_pt->entries[pt_idx] &= ~PAGE_WRITE;

                    uint32_t phys_addr = parent_pt->entries[pt_idx] & 0xFFFFF000;
                    pmm_increment_ref_count(phys_addr);
                }
            }

        }
    }
    __asm__ volatile("mov %0, %%cr3"::"r"((uint32_t)parent_pd):"memory");
}

void vmm_map_page_to_dir(page_directory_t* pd,uint32_t virtual_addr,uint32_t physical_addr, uint32_t flags){
    uint32_t pd_index = virtual_addr >>22;
    uint32_t pt_index = (virtual_addr>>12)&0x03FF;

    if(!(pd->entries[pd_index]&PAGE_PRESENT)){
        uint32_t new_table_phys = (uint32_t)pmm_alloc_frame();
        page_table_t* new_table = (page_table_t*)new_table_phys;

        memset(new_table,0,4096);

        pd->entries[pd_index] = new_table_phys|PAGE_PRESENT|PAGE_WRITE|PAGE_USER;
    }
    uint32_t table_phys = pd->entries[pd_index]&0xFFFFF000;
    page_table_t* pt = (page_table_t*)table_phys;

    pt->entries[pt_index] = (physical_addr&0xFFFFF000)|PAGE_PRESENT|flags;
}

void handle_cow_fault(uint32_t fault_addr){
    //page boundary where fault has occured
    uint32_t page_aligned = fault_addr & 0xFFFFF000;

    uint32_t pd_index = page_aligned>>22;
    uint32_t pt_index = (page_aligned>>12)&0x03FF;
    uint32_t table_phys = current_task->page_directory->entries[pd_index] & 0xFFFFF000;
    page_table_t* pt = (page_table_t*)table_phys;

    uint32_t old_phys_frame = pt->entries[pt_index]&0xFFFFF000;
    //reading memory to kernel buffer
    memcpy(cow_temp_buffer,(void*)page_aligned,4096);

    uint32_t new_phys_frame = (uint32_t)pmm_alloc_frame();

    pt->entries[pt_index] = new_phys_frame|PAGE_PRESENT|PAGE_WRITE|PAGE_USER;

    __asm__ volatile ("invlpg (%0)"::"r"(page_aligned):"memory");
    memcpy((void*)page_aligned,cow_temp_buffer,4096);

    //decrement reference count of the old shared frame
    pmm_free_frame((void*)old_phys_frame);
}