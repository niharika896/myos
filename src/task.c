#include "task.h"
#include "heap.h"
#include "vga.h"
#include "gdt.h"

struct process tasks[TASKS];
struct process* current_task;

extern struct tss_entry_struct tss_entry;
extern void switch_context(struct context** old, struct context* new);

void task_entry(){
    __asm__ volatile("sti"); //enable interrupts
    current_task->entry_point();
    for(;;){yield();}
}

uint32_t create_task(void (*entry_point)()){
    uint32_t pid = 0;
    
    for (int i = 1; i < TASKS; i++) {
        if (tasks[i].state == UNUSED || tasks[i].state == DEAD) {
            pid = i;
            break;
        }
    }        
    if (pid == 0) {
        return 0; 
        }

    tasks[pid].pid = pid;
    tasks[pid].state = RUNNABLE;
    tasks[pid].entry_point = entry_point;

    tasks[pid].kernel_stack = (uint8_t*)kmalloc(4096);

    tasks[pid].files[0] = 0; // STDIN  (Keyboard)
    tasks[pid].files[1] = 1; // STDOUT (VGA)
        
    // -1 =>closed
    for (int i = 2; i < 16; i++) {
        tasks[pid].files[i] = -1;
    }
    //bottom of the stack as stack grows downwards
    uint32_t* stack = (uint32_t*)(tasks[pid].kernel_stack + 4096);

    *(--stack) = (uint32_t)task_entry;
    *(--stack) = 0;
    *(--stack) = 0;
    *(--stack) = 0;
    *(--stack) = 0;

    tasks[pid].context = (struct context*)stack;
    return pid;
}

void tasking_init(void){
    tasks[0].pid = 0;
    tasks[0].state = RUNNING;
    tasks[0].kernel_stack = (uint8_t*)kmalloc(4096);
    current_task = &tasks[0];
}

void yield(void){

    __asm__ volatile("cli");
    uint32_t next_pid = (current_task->pid+1)%TASKS;

    while(tasks[next_pid].state!=RUNNABLE && next_pid!=current_task->pid){
        next_pid = (next_pid +1)%TASKS;
    }
    if(next_pid == current_task->pid && current_task->state == RUNNING){
        __asm__ volatile("sti");
        return;
    }
    struct process* next_task = &tasks[next_pid];
    struct process* previous_task = current_task;
    if(previous_task->state == RUNNING){
        previous_task->state = RUNNABLE;
    }
    next_task->state = RUNNING;
    current_task=next_task;
    tss_entry.esp0 = (uint32_t)next_task->kernel_stack + 4096;
    switch_context(&previous_task->context, next_task->context);
    __asm__ volatile("sti");
}