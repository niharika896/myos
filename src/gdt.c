#include "gdt.h"

// These MUST be global variables so they don't get destroyed on the stack
struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct   gdt_ptr;

struct tss_entry_struct tss_entry;

// The external assembly function
extern void gdt_flush(uint32_t);

void tss_set_stack(uint32_t kernel_esp) {
    tss_entry.esp0 = kernel_esp;
}

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

void write_tss(int32_t num,uint16_t ss0,uint32_t esp0){
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = sizeof(tss_entry)-1;

    gdt_set_gate(num,base,limit,0xE9,0x00);

    uint8_t *tss_bytes = (uint8_t*)&tss_entry;
    for(uint32_t i=0;i<sizeof(tss_entry);i++){
        tss_bytes[i]=0;
    }
    tss_entry.ss0=ss0;
    tss_entry.esp0=esp0;

    tss_entry.iomap_base = sizeof(tss_entry);
}

void init_gdt(void) {
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    // Null segment
    gdt_set_gate(0, 0, 0, 0, 0); 
    //Kernel Code segment (Offset 0x08)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); 
    //Kernel Data segment (Offset 0x10)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); 
    //User Code Segment (Ring 3, Offset 0x18)
    //access 0xFA
    gdt_set_gate(3,0,0xFFFFFFFF, 0xFA, 0xCF);

    //User Data Segment (Ring 3, Offset 0x20)
    //access 0xF2
    gdt_set_gate(4,0,0xFFFFFFFF,0xF2,0xCF);

    //TSS (Offset 0x28)
    write_tss(5,0x10,0x0);

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}