
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

