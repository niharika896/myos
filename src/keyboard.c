#include "io.h"
#include "pic.h"
#include <stdint.h>
#include "vga.h"

//QWERTY Scancode to ASCII lookup table
const char lookup_table[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
  '*', 0,  ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    //check for press
    if (!(scancode & 0x80)) {
        char c =lookup_table[scancode];
        if (c != 0) {
            terminal_putchar(c);
        }
    }
    pic_send_eoi(1);
}