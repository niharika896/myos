#include <stdint.h>
#include "vga.h"

void print_hex(uint32_t num) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[11]; // "0x" + 8 digits + null
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[10] = '\0';
    
    for (int i = 7; i >= 0; i--) {
        buffer[i + 2] = hex_chars[num & 0xF];
        num >>= 4;
    }
    terminal_writestring(buffer);
    terminal_writestring("\n");
}