#include "idt.h"
#include "helpers.h"
#include "keyboard.h"
extern void terminal_writestring(const char* data);

void isr_handler(struct registers* regs) {
    #include "timer.h"
    
    if (regs->int_no == 32) {
        timer_handler();
        return;
    }
    if(regs->int_no == 33){
        keyboard_handler();
        return;
    } 

    terminal_writestring("CPU EXCEPTION OCCURRED: ");
    
    if (regs->int_no == 0) {
        terminal_writestring("Division by Zero Error!\n");
    } else if (regs->int_no == 13) {
        terminal_writestring("General Protection Fault!\n");
    } else if (regs->int_no == 14) {
        uint32_t faulting_address;
        __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));
        
        terminal_writestring("\n*** PAGE FAULT ***\n");
        
        terminal_writestring("Faulting Address: ");
        print_hex(faulting_address);

        // Analyze the error code pushed by the CPU
        terminal_writestring("Reason: ");
        int present   = !(regs->err_code & 0x1); // Bit 0: 0 = Not Present, 1 = Protection Violation
        int rw        = regs->err_code & 0x2;    // Bit 1: 0 = Read, 1 = Write
        int us        = regs->err_code & 0x4;    // Bit 2: 0 = Kernel, 1 = User
        
        if (present) terminal_writestring("Page not present. ");
        if (rw) terminal_writestring("Write operation. ");
        else terminal_writestring("Read operation. ");
        if (us) terminal_writestring("In User Mode. ");
        else terminal_writestring("In Kernel Mode. ");
    }else {
        terminal_writestring("Unhandled Exception!\n");
    }
    // Freeze the CPU on crash
    for (;;) {
        __asm__ volatile ("hlt");
    }
}