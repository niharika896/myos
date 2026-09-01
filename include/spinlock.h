#ifndef SPINLOCK_H
#define SPINLOCK_H
#include <stdint.h>

typedef struct{
    uint32_t locked;
}spinlock_t;

static inline void spinlock_init(spinlock_t* lock){
    lock->locked = 0;
}

static inline void spinlock_acquire(spinlock_t* lock){

    __asm__ volatile("cli");
    while(1){
        uint32_t result;
        __asm__ volatile("lock xchg %0, %1":"=r"(result),"+m"(lock->locked):"0"(1):"memory");

        if(result == 0)break;
    } 
}

static inline void spinlock_release(spinlock_t* lock){
    lock->locked = 0;

    __asm__ volatile("sti");
}
#endif