#include "vga.h"
#include "task.h"
#include "helpers.h"
#include <stdint.h>

//pusha formal
struct registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

void syscall_handler(struct registers* regs) {

    if (regs->eax == 4) {  //sys_write
        int fd = regs->ebx;
        char* str = (char*)regs->ecx;
        
        if (fd >= 0 && fd < 16 && current_task->files[fd] == 1) {
            terminal_writestring(str);
            regs->eax = 0; 
        } else {
            regs->eax = -1; 
        }
    } 
    else if (regs->eax == 3) {  //sys_read
        int fd = regs->ebx;
        char* buf = (char*)regs->ecx;

        if (fd >= 0 && fd < 16 && current_task->files[fd] == 0) {
            kgets(buf);   
            regs->eax = 0;  
        } else {
            regs->eax = -1; 
        }
    }
    else if (regs->eax == 1) { // sys_exit
        current_task->state = DEAD;
        yield();
    } 
    else if (regs->eax == 158) { // sys_yield
        yield();
    } 
}