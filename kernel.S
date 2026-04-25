//
// kernel.S
//

.section .text
.globl main
main:
    bl LED_off
    mov w0, #0x3F0000
    bl wait
    bl LED_on
    mov w0, #0x3F0000
    bl wait
    b main
