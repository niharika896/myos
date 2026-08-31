#ifndef IO_H
#define IO_H

#include <stdint.h>
//a means registers, Nd means 8bit add or if its larger address store in dx
static inline void outb(uint16_t port, uint8_t val){
    __asm__ volatile ("outb %0, %1": : "a"(val), "Nd"(port));

}

static inline uint8_t inb(uint16_t port){
    uint8_t ret;
    __asm__ volatile ("inb %1, %0": "=a"(ret):"Nd"(port));
    return ret;
}
//delay
static inline void io_wait(void){
    outb(0x80,0);
}
#endif