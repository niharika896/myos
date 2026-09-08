#include "idt.h"
#include "helpers.h"
#include "keyboard.h"
#include "paging.h"
#include "task.h"
#include "timer.h"
extern void terminal_writestring(const char* data);

extern uint32_t sys_fork(struct registers* regs);

void isr_handler(struct registers* regs) {
    if (regs->int_no == 32) { 
        timer_handler();
        return;
    }
    if (regs->int_no == 33) {
        keyboard_handler(); 
        return; 
    } 
    
    if(regs->int_no == 128) {
        if(regs->eax == 2) { 
            // sys_fork
            regs->eax = sys_fork(regs);
        } 
        else if (regs->eax == 4) { 
            // sys_write
            terminal_writestring((char*)regs->ecx); 
        } 
        else if (regs->eax == 1) { 
            //sys_exit
            tasks[current_task->pid].state = DEAD;
            yield(); 
        }
        return;
    }
    
    if (regs->int_no == 14) {
        uint32_t faulting_address;
        __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));
        
        int is_present = regs->err_code & 0x01;
        int is_write = regs->err_code & 0x02;

        if (is_present && is_write) {
            handle_cow_fault(faulting_address);
            return; 
        } else {
            terminal_writestring("\nSegmentation Fault! Killing process.\n");
            tasks[current_task->pid].state = DEAD;
            yield();
            return;
        }
    }

    // --- Fatal Crashes ---
    terminal_writestring("\nCPU EXCEPTION OCCURRED: ");
    if (regs->int_no == 0) terminal_writestring("Division by Zero Error!\n");
    else if (regs->int_no == 13) terminal_writestring("General Protection Fault!\n");
    else terminal_writestring("Unhandled Exception!\n");
    
    for (;;) { __asm__ volatile ("hlt"); }
}