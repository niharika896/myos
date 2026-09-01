#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "multiboot.h"
#include "vga.h"
#include "pmm.h"
#include "helpers.h"
#include "paging.h"
#include "heap.h"
#include "pic.h"
#include "timer.h"
#include "task.h" 

// /* Check if the compiler thinks you are targeting the wrong operating system. */
// #if defined(__linux__)
// #error "You are not using a cross-compiler, you will most certainly run into trouble"
// #endif

// /* This tutorial will only work for the 32-bit ix86 targets. */
// #if !defined(__i386__)
// #error "This tutorial needs to be compiled with a ix86-elf compiler"
// #endif

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

void task_a() {
    for (;;) {
        terminal_writestring("A");
        // No yield! I am hogging the CPU!
        for (volatile int i = 0; i < 10000000; i++) {}
    }
}

void task_b() {
    for (;;) {
        terminal_writestring("B");
        // No yield! I am hogging the CPU!
        for (volatile int i = 0; i < 10000000; i++) {}
    }
}

void kernel_main(uint32_t magic, uint32_t addr) {
    init_gdt();
    init_idt();

    pic_remap();

    terminal_initialize();

    if (magic != MULTIBOOT_MAGIC) {
        terminal_writestring("Error: Invalid Multiboot Magic Number!\n");
        for(;;) { __asm__ volatile("hlt"); }
    }

    
    terminal_writestring("Multiboot Magic Number verified.\n");

    pmm_init();

    multiboot_info_t* mbi = (multiboot_info_t*) addr;

    // Check if bit 6 of flags is set (this means the memory map is valid)
    if (mbi->flags & (1 << 6)) {
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) mbi->mmap_addr;
        
        // Loop through the memory map using the length provided by GRUB
        while ((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length) {
            
            // mmap->type == 1 means this is usable RAM!
            if (mmap->type == 1) {
                pmm_mark_free(mmap->addr_low, mmap->len_low);
            }

            // Move to the next entry in the map
            mmap = (multiboot_memory_map_t*) ((uint32_t)mmap + mmap->size + sizeof(mmap->size));
        }
    }
    uint32_t kernel_start_addr = (uint32_t)&_kernel_start;
    uint32_t kernel_end_addr = (uint32_t)&_kernel_end;
    uint32_t kernel_size = kernel_end_addr - kernel_start_addr;

    pmm_mark_used(kernel_start_addr, kernel_size);

    terminal_writestring("PMM initialised\n");

    init_paging();
    terminal_writestring("Paging Enabled\n");

    init_heap();
    terminal_writestring("Heap Initialised");


    init_timer(100);
    
    terminal_writestring("Hardware Interrupts Enabled!\n");

    tasking_init();
    create_task(1, task_a);
    create_task(2, task_b);

    // Turn on interrupts so the keyboard still works
    __asm__ volatile("sti");

    // The Main Kernel becomes Task 0's infinite loop
    for(;;) {
        terminal_writestring("0");
        // yield(); 
        for (volatile int i = 0; i < 10000000; i++) {}
    }
    // waiting for input
    for(;;) {
        __asm__ volatile("hlt"); 
    }
}