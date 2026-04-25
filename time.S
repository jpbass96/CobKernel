//
// time.S
//

// void wait(int loop -> w0)
.section .text
.globl wait
wait:
    str x30, [sp, -16]!
    mov w1, 0
1:
    cmp w1, w0
    bgt 2f
    add w1, w1, 1
    b 1b
2:
    ldr x30, [sp], 16
    ret
