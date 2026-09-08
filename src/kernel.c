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
#include "keyboard.h"
#include "libc.h"

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

// --- Ring 3 "Standard Library" ---
void sys_write(int fd, char* str, int len) {
    int ret;
    // "=a"(ret) forces GCC to realize EAX is destroyed by the kernel!
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(4), "b"(fd), "c"(str), "d"(len) : "memory");
}

int sys_read(int fd, char* buf, int len) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(3), "b"(fd), "c"(buf), "d"(len) : "memory");
    return ret;
}

void sys_yield() {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(158) : "memory");
}

void sys_exit() {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(1) : "memory");
}

void user_program() {
    int pid = fork();
    
    if (pid == 0) {
        char child_msg[] = "Child: Woke up in cloned memory!\n";
        sys_write(1, child_msg, 33);
        
        // This variable assignment triggers Exception 14 (CoW Trap)
        // The hardware will duplicate the physical frame in the background.
        int child_var = 100; 
        
        sys_exit();
    } else {
        char parent_msg[] = "Parent: Successfully forked!\n";
        sys_write(1, parent_msg, 29);
        
        // This variable assignment triggers Exception 14 (CoW Trap)
        int parent_var = 200;
        
        sys_exit();
    }
}

void task_b() {
    terminal_writestring("\n[Kernel] Allocating true isolated user stack...\n");

    uint32_t user_stack_phys = (uint32_t)pmm_alloc_frame();

    uint32_t user_stack_virt = 0x80000000;

    vmm_map_page_to_dir(current_task->page_directory, user_stack_virt, user_stack_phys, PAGE_USER | PAGE_WRITE);
    //flushing tlb
    __asm__ volatile("mov %0, %%cr3" : : "r"((uint32_t)current_task->page_directory) : "memory");
    uint32_t user_stack_bottom = user_stack_virt + 4096;
    
    terminal_writestring("[Kernel] Dropping privileges into isolated Ring 3...\n");
    
    jump_usermode((uint32_t)user_program, user_stack_bottom);
}

void task_a() {
    char input[256];
    for(;;){
        terminal_writestring("myOS> ");
        kgets(input);

        if(strcmp(input,"help") == 0){
            terminal_writestring("Commands: help, clear,ping\n");
        }else if(strcmp(input,"clear")==0){
            terminal_initialize();
        }
        else if(strcmp(input,"ping")==0){
            terminal_writestring("pong!\n");
        }
        else if(strcmp(input,"sandbox")==0){
            uint32_t child_pid = create_task(task_b);
            
            if (child_pid != 0) {
                while (tasks[child_pid].state != DEAD && tasks[child_pid].state != UNUSED) {
                    yield(); 
                }
                pmm_free_frame((void*)tasks[child_pid].page_directory);
                kfree(tasks[child_pid].kernel_stack);
                tasks[child_pid].state = UNUSED;
            } else {
                terminal_writestring("Error: Maximum processes reached.\n");
            }
        }
        else if(input[0]!='\0'){
            terminal_writestring("Unknown command: ");
            terminal_writestring(input);
            terminal_writestring("\n");
        }
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
    create_task(task_a);
    // create_task(2, task_b);

    // Turn on interrupts so the keyboard still works
    __asm__ volatile("sti");
    // waiting for input
    for(;;) {
        __asm__ volatile("hlt"); 
    }
}