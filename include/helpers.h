#include <stdint.h>
#ifndef HELPERS_H
#define HELPERS_H


void print_hex(uint32_t num);
void terminal_write_uint(uint32_t n);
void* memset(void* dest, int val, uint32_t count) ;
void memcpy(void* dest,const void* src,uint32_t count);
#endif