//
// config.h
//

#ifndef _config_h
#define _config_h

.equ MEGABYTE, 0x100000

.equ MEM_KERNEL_START, 0x80000 // Start address of the main program
.equ KERNEL_MAX_SIZE, (2 * MEGABYTE)
.equ MEM_KERNEL_END, (MEM_KERNEL_START + KERNEL_MAX_SIZE)
.equ KERNEL_STACK_SIZE, 0x20000
.equ MEM_KERNEL_STACK, (MEM_KERNEL_END + KERNEL_STACK_SIZE)

#endif
