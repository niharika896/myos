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

void terminal_write_uint(uint32_t n) {
    char buf[11]; // max "4294967295" + null terminator
    int i = 10;
    buf[i] = '\0';

    if (n == 0) {
        terminal_writestring("0");
        return;
    }

    while (n > 0 && i > 0) {
        buf[--i] = '0' + (n % 10);
        n /= 10;
    }

    terminal_writestring(&buf[i]);
}