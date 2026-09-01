.global switch_context
.type switch_context, @function

# void switch_context(struct context **old, struct context *new);
switch_context:
    mov 4(%esp), %eax    // %eax = pointer to the old context pointer
    mov 8(%esp), %edx    // %edx = the new context pointer

    push %ebp
    push %ebx
    push %esi
    push %edi

    mov %esp, (%eax)

    mov %edx, %esp

    pop %edi
    pop %esi
    pop %ebx
    pop %ebp

    ret