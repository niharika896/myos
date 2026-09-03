#include <stdint.h>

struct tss_entry_struct{
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax,ecx,edx,ebx,esp,ebp,esi,edi;
    uint32_t es,cs,ss,ds,fs,gs;
    uint32_t ldt;
    uint32_t trap;
    uint16_t iomap_base;
}__attribute__((packed));

extern void tss_flush(void);

struct gdt_entry_struct{
    uint16_t limit_low; //lower 16 bits of limit
    uint16_t base_low; //lower 16 bits of base
    uint8_t base_middle; //next 8 bits of base
    uint8_t access; //ring level
    uint8_t granularity; //granularity flags and upper 4 bits of limit
    uint8_t base_high; //last 8 bits of base
}__attribute__((packed));

struct gdt_ptr_struct{
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));

void init_gdt(void);

void tss_set_stack(uint32_t kernel_esp);