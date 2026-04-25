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
