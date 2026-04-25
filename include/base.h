// base.h
//

#ifndef _base_h
#define _base_h

.equ RPI_BASE, 0x107C000000UL

// GPIO
.equ ARM_GPIO2_BASE, RPI_BASE + 0x1517C00
.equ ARM_GPIO2_DATA0, ARM_GPIO2_BASE + 0x04

#endif
