#include "timer.h"
#include "io.h"
#include "pic.h"
#include <stdint.h>

volatile uint32_t timer_ticks = 0;

void init_timer(uint32_t frequency) {
    // The hardware clock runs at 1193180 Hz
    uint32_t divisor = 1193180 / frequency;

    // Send the command byte (Set repeating mode)
    outb(0x43, 0x36);

    // Send the divisor byte by byte (Low byte, then High byte)
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void timer_handler(void) {
    timer_ticks++;
    pic_send_eoi(0);
}