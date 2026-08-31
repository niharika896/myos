#include <stdint.h>

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
