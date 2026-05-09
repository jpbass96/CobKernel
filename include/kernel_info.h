#ifndef _kernel_info_h
#define _kernel_info_h

#include "kernel_info.h"
#include "types.h"

void print_cpsr(u8 elevel);
void print_scr_el3();
u64 get_core_affinity();
void get_core_context();


#endif
