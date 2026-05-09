// util.s

.globl write32
write32:
    str w1, [x0]
    ret

.globl read32
read32:
    ldr w0, [x0]
    ret
