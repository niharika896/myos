#include "io.h"
#include "pic.h"
#include <stdint.h>
#include "vga.h"
#include "task.h"

//QWERTY Scancode to ASCII lookup table
const char lookup_table[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
  '*', 0,  ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define KBD_BUFFER_SIZE 256
char kbd_buffer[KBD_BUFFER_SIZE];
uint32_t kbd_index=0;
volatile int line_ready = 0;

void kgets(char* output_buffer){
  line_ready = 0;
  kbd_index = 0;

  current_task->state = SLEEPING;
  yield();

  for(int i=0;i<=kbd_index;i++){
    output_buffer[i] = kbd_buffer[i];
  }

}

int strcmp(const char* s1, const char* s2){
  while(*s1 && (*s1==*s2)){
    s1++;
    s2++;
  }
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    //check for press
    if (!(scancode & 0x80)) {
        char c =lookup_table[scancode];
        if (c != 0) {
            // terminal_putchar(c);
            if(c == '\b'){
              if(kbd_index > 0){
                kbd_index--;        
                terminal_putchar('\b');
              }
            }else if(c == '\n'){
              terminal_putchar('\n');
              kbd_buffer[kbd_index] = '\0';
              line_ready = 1;

              tasks[1].state = RUNNABLE;
            }else{
              if(kbd_index<KBD_BUFFER_SIZE -1){
                kbd_buffer[kbd_index++] = c;
                terminal_putchar(c);
              }
            }
        }
    }
    
    pic_send_eoi(1);
}