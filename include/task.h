#ifndef TASK_H
#define TASK_H
#include <stdint.h>

#define TASKS 3

struct context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip; // instruction pointer
};

enum process_state { UNUSED, RUNNABLE, RUNNING, SLEEPING, DEAD};

struct process {
    uint32_t pid;
    enum process_state state;
    uint32_t* page_directory; //memory map
    uint8_t* kernel_stack;     // stack for this process
    struct context* context;
    void (*entry_point)();
    //file descriptor table
    int files[16];
};

extern struct process tasks[TASKS];
extern struct process* current_task;

void tasking_init(void);

uint32_t create_task(void (*entry_point)());
void yield(void); // forcing the current process to give up the CPU
void kgets(char* output_buffer);

extern void jump_usermode(uint32_t entry_point, uint32_t user_stack);

#endif