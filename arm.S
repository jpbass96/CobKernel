//TODO  update this to use offsetof later
.equ ARM_SMCCC_RES_X0_OFFS, (0)
.equ ARM_SMCCC_RES_X2_OFFS, (16)

//Macro allowing for either hvc or smc smccc instructions
.macro SMCCC instr
    //run SMMC instruction assuming x0-x7 registers are prepopulated by function call
    \instr  #0 
    //load x4 with stack pointer. This holds the 9th function argument (resource struct pointer)
    ldr     x4, [sp]
    //load x0/x1 to the first 16 bytes of the resource struct
    stp     x0, x1, [x4, #ARM_SMCCC_RES_X0_OFFS]
    //load x2/x3 to the 2nd 16 bytes of the resource struct
    stp     x2, x3, [x4, #ARM_SMCCC_RES_X2_OFFS]
    ret
    .endm

/*
 * void arm_smccc_smc(unsigned long a0, unsigned long a1, unsigned long a2,
 *                unsigned long a3, unsigned long a4, unsigned long a5,
 *                unsigned long a6, unsigned long a7, struct arm_smccc_res *res,
 *                 )
 */
 .global arm_smccc_smc
 .align 8
arm_smccc_smc:
        SMCCC   smc

    
//void arm64_take_semaphore_exclusive(semaphore sem, u8 wait);
.global arm64_take_semaphore_exclusive
arm64_take_semaphore_exclusive:
    ldaxr x1, [x0] //trying to take exclusive lock
    cbnz x1, _arm64_sem_spin //if already taken, then spin
    mov x1, #1 //if not already taken, load value with 1
    stlxr w2, x1, [x0] //try to write to the lock
    dmb ish
    cbnz w2, _arm64_sem_spin
    ret
    _arm64_sem_spin: 
    wfe
    b arm64_take_semaphore_exclusive

//void arm64_put_semaphore_exclusive(semaphore sem);
//does this need checking to see if the sem was actually taken?
.global arm64_put_semaphore_exclusive
arm64_put_semaphore_exclusive:
    dmb sy //making sure whatever needed to happen happens before releasing the semaphore
    ldaxr x1, [x0]
    //use exclusive store to set the semaphore back to 0
    mov x1, #0
    stlxr w2, x1, [x0] //try to write to the lock
    dmb ish
    //if the store wasnt successful, retry. This should be an exception
    //This condition failing is indicative of a software bug. It indicates a thread
    //released a semaphore it didnt already own.
    cmp w2, #0
    bne arm64_put_semaphore_exclusive
    sev
    ret

//void arm64_init_semaphore(semaphore sem)
.global arm64_init_semaphore
arm64_init_semaphore:
    ldaxr x1, [x0]
    mov x1, #0
    stlxr w2, x1, [x0] //try to write to the lock
    dmb ish
    mov w0, w2
    ldr x2, [x0]
    cmp w2, #0
    bne arm64_init_semaphore
    mov w0, w2
    ret
  


