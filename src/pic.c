#include "pic.h"
#include "io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_EOI      0x20

// Remap the hardware interrupts to start at 32 (0x20) and 40 (0x28)
void pic_remap(void) {
    // unsigned char a1, a2;

    // Save the current masks
    // a1 = inb(PIC1_DATA);
    // a2 = inb(PIC2_DATA);

    // Start the initialization sequence in cascade mode
    outb(PIC1_COMMAND, 0x11);
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    // ICW2: Vector offsets (Tell Master PIC to start at 32, Slave at 40)
    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();

    // ICW3: Tell Master there is a Slave PIC at IRQ2
    outb(PIC1_DATA, 4);
    io_wait();
    // Tell Slave its cascade identity
    outb(PIC2_DATA, 2);
    io_wait();

    // ICW4: Set 8086/88 (x86) mode
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    // uncomment for no interrupts
    // outb(PIC1_DATA, a1);
    // outb(PIC2_DATA, a2);

    //comment out for no interrupts
    // 0xFD is binary 11111101. 
    //blocks everything on the Master PIC except bit 1 which represents IRQ1 (Keyboard).
    outb(PIC1_DATA, 0xFC); 
    
    //completely blocks the Slave PIC.
    outb(PIC2_DATA, 0xFF);
}

// Tell the PIC we finished handling the interrupt
void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI); //slave
    }
    outb(PIC1_COMMAND, PIC_EOI); //master
}