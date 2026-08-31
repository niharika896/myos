.section .text
.align 4

.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax    # Get the pointer passed from C
    lgdt (%eax)          # Load the new GDT pointer

    # Reload data segment registers
    mov $0x10, %ax       # 0x10 points at the new data selector
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # Perform a far jump to reload the Code Segment (CS) register
    ljmp $0x08, $flush_cs

flush_cs:
    ret