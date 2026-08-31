#ifndef MULTIBOOT_H
#define MULTIBOOT_H
#include <stdint.h>

#define MULTIBOOT_MAGIC 0x2BADB002

typedef struct multiboot_memory_map {
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
}__attribute__((packed)) multiboot_memory_map_t;

typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
    uint32_t mmap_length; // Total size of the memory map
    uint32_t mmap_addr;   // Memory address of the first entry
} __attribute__((packed)) multiboot_info_t;

#endif