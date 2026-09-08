#include "libc.h"

int fork() {
    int ret_val;
    __asm__ volatile (
        "mov $2, %%eax\n" 
        "int $0x80\n"
        : "=a" (ret_val)
    );
    return ret_val;
}