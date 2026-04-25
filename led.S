//
// led.S
//

#include "base.h"

// void LED_off(void) // Turns off the LED
.section .text
.globl LED_off
LED_off:
    ldr x1, =ARM_GPIO2_DATA0
    ldr w0, [x1]
    bic w0, w0, #0x200
    str w0, [x1]
    ret

// void LED_on(void) // Turns on the LED
.section .text
.globl LED_on
LED_on:
    ldr x1, =ARM_GPIO2_DATA0
    ldr w0, [x1]
    orr w0, w0, #0x200
    str w0, [x1]
    ret
