//
// boot.S
//

#include "config.h"

.section .init  // Ensure the linker places this at the beginning of the kernel image
.global _start  // Execution starts here

_start:
	ldr	x0, =MEM_KERNEL_STACK
	mov	sp, x0			// init its stack	
	b sysinit

//for now assume context id will just be core idx for use in setting up stack
//Do not change x0 register. main will use this to identify its core index
.section .text
.global _secondary_start
_secondary_start:
	ldr x1, =MEM_KERNEL_STACK //load register 1 with kernel stack start
	ldr x2, =KERNEL_STACK_SIZE //load register 2 with kernel stack size
	mul x2, x0, x2 //store x2 with stack offset for this core
	add x1, x1, x2 //add stack offset to kernel stack start
	mov sp, x1 //setup stack pointer
	b sysinit
	 