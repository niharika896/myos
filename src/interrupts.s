.section .text
.align 4

.global idt_flush
.type idt_flush, @function
idt_flush:
    mov 4(%esp), %eax
    lidt (%eax)
    ret
.extern isr_handler

# Macro for exceptions that do NOT push an error code (push dummy 0)
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    push $0            //Push dummy error code
    push $\num         //Push interrupt number
    jmp isr_common_stub
.endm

# Macro for exceptions that DO push an error code automatically
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    push $\num         //Push interrupt number
    jmp isr_common_stub
.endm

# Define CPU Exceptions (0-31)
ISR_NOERRCODE 0  //0: Divide By Zero
ISR_NOERRCODE 1  //1: Debug
ISR_NOERRCODE 2  //2: Non-Maskable Interrupt
ISR_NOERRCODE 3  //3: Breakpoint
ISR_NOERRCODE 4  //4: Overflow
ISR_NOERRCODE 5  //5: Bound Range Exceeded
ISR_NOERRCODE 6  //6: Invalid Opcode
ISR_NOERRCODE 7  //7: Device Not Available
ISR_ERRCODE   8  //8: Double Fault (pushes error code)
ISR_NOERRCODE 9  //9: Coprocessor Segment Overrun
ISR_ERRCODE   10 //10: Invalid TSS (pushes error code)
ISR_ERRCODE   11 //11: Segment Not Present (pushes error code)
ISR_ERRCODE   12 //12: Stack-Segment Fault (pushes error code)
ISR_ERRCODE   13 //13: General Protection Fault (pushes error code)
ISR_ERRCODE   14 //14: Page Fault (pushes error code)
ISR_NOERRCODE 15 //15: Reserved
ISR_NOERRCODE 16 //16: x87 Floating-Point Exception
ISR_ERRCODE   17 //17: Alignment Check (pushes error code)
ISR_NOERRCODE 18 //18: Machine Check
ISR_NOERRCODE 19 //19: SIMD Floating-Point Exception
ISR_NOERRCODE 20 //20: Virtualization Exception
ISR_ERRCODE   21 //21: Control Protection Exception (pushes error code)
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_ERRCODE   29 //29: VMM Communication Exception
ISR_ERRCODE   30 //30: Security Exception
ISR_NOERRCODE 31

.global isr32
isr32:
    cli
    push $0       # Dummy error code
    push $32      # Interrupt 32 (IRQ0 Timer)
    jmp isr_common_stub

.global isr33
isr33:
    cli
    push $0
    push $33
    jmp isr_common_stub 

# Common ISR stub: saves CPU state, calls C handler, restores state
isr_common_stub:    
    pusha               //Pushes edi, esi, ebp, esp, ebx, edx, ecx, eax

    mov %ds, %ax        //Lower 16-bits of eax = ds
    push %eax           //Save the data segment descriptor

    mov $0x10, %ax      //Load the kernel data segment descriptor (0x10)
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp           //Pass pointer to stack (struct registers*) to C
    call isr_handler
    add $4, %esp        //Clean up pushed pointer

    pop %eax            //Reload original data segment descriptor
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa                //Pops edi, esi, ebp, esp, ebx, edx, ecx, eax
    add $8, %esp        //Cleans up the pushed error code and pushed ISR number
    iret               //Return from interrupt

.global isr128
.type isr128, @function
isr128:
    cli
    pusha //all gprs 
    push %esp 

    call syscall_handler

    add $4, %esp
    popa
    iret    //return to ring 3