#include "task.h"
#include "heap.h"
#include "vga.h"

#define TASKS 3

struct process tasks[TASKS];
struct process* current_task;

extern void switch_context(struct context** old, struct context* new);

void task_entry(){
    __asm__ volatile("sti"); //enable interrupts
    current_task->entry_point();
    for(;;){yield();}
}

void create_task(uint32_t pid, void (*entry_point)()){
    tasks[pid].pid = pid;
    tasks[pid].state = RUNNABLE;
    tasks[pid].entry_point = entry_point;

    tasks[pid].kernel_stack = (uint8_t*)kmalloc(4096);

    //bottom of the stack as stack grows downwards
    uint32_t* stack = (uint32_t*)(tasks[pid].kernel_stack + 4096);

    *(--stack) = (uint32_t)task_entry;
    *(--stack) = 0;
    *(--stack) = 0;
    *(--stack) = 0;
    *(--stack) = 0;

    tasks[pid].context = (struct context*)stack;
}

void tasking_init(void){
    tasks[0].pid = 0;
    tasks[0].state = RUNNING;
    current_task = &tasks[0];
}

void yield(void){
    uint32_t next_pid = (current_task->pid+1)%TASKS;

    while(tasks[next_pid].state!=RUNNABLE && next_pid!=current_task->pid){
        next_pid = (next_pid +1)%TASKS;
    }
    if(next_pid == current_task->pid && current_task->state == RUNNING){
        return;
    }
    struct process* next_task = &tasks[next_pid];
    struct process* previous_task = current_task;
    previous_task->state = RUNNABLE;
    next_task->state = RUNNING;
    current_task=next_task;
    switch_context(&previous_task->context, next_task->context);
}