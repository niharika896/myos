# Declare constants for the multiboot header
.set ALIGN,    1<<0             
.set MEMINFO,  1<<1             
.set FLAGS,    ALIGN | MEMINFO  
.set MAGIC,    0x1BADB002       
.set CHECKSUM, -(MAGIC + FLAGS) 

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# bss is for uninitialized data, doesnt take up actual space in the disk
.section .bss 
.align 16  
stack_bottom:
.skip 16384 
stack_top: 

# _start is the entry point
.section .text 
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp
    #ebx is the pointer to the multiboot info structure
    push %ebx
    # eax is the magic value that tells us that grub has booted us
    push %eax
    call kernel_main

    cli
1:  hlt
    jmp 1b

.size _start, . - _start