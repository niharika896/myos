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

.global tss_flush
.type tss_flush, @function
tss_flush:
    mov $0x2B, %ax
    ltr %ax
    ret;

.global jump_usermode
.type jump_usermode, @function
jump_usermode:
    cli
    mov 4(%esp),%eax
    mov 8(%esp),%edx

    mov $0x23, %cx 
    mov %cx, %ds
    mov %cx, %es
    mov %cx, %fs
    mov %cx, %gs

    push $0x23 //push user data segment
    push %edx //push user stack pointer

    pushfl
    pop %ebx
    or $0x200, %ebx //set interrupt flag
    push %ebx //push modified flags

    push $0x1B //push user code segment
    push %eax //push eip

    iret

