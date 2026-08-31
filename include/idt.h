#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry_struct{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
}__attribute__((packed));

struct idt_ptr_struct{
    uint16_t limit;
    uint32_t base;  
}__attribute__((packed));

struct registers {
    uint32_t ds;                                           // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;     // Pushed by pusha
    uint32_t int_no, err_code;                             // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;                // Pushed by the CPU automatically
}__attribute__((packed));

void init_idt(void);

void idt_set_gate(uint8_t num,uint32_t base,uint16_t sel, uint8_t flags);

extern void idt_flush(uint32_t ptr);

#endif