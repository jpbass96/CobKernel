//
// sysinit.S
//

#include "config.h"

.section .text
.global sysinit
sysinit:
    ldr x9, =vector_table_el2
    msr VBAR_EL2, x9 //setup vector table for EL2
    str x0, [sp, #-16]!
    cmp x0, #0
    b.ne _skip_table_setup
    bl setup_early_mmu_tables
    _skip_table_setup: bl setup_early_mmu
    ldr x0, [sp], #16
    mov x1, ARM_MMU_TABLE_BASE_LO
    mov x2, HEAP_START
    mov x3, HEAP_END
    b main


// Typical exception vector table code.
.balign 0x800
vector_table_el2:
curr_el_sp0_sync: // The exception handler for the synchronous
                 // exception from the current EL using SP0.
    mov x0, #1
    b handle_generic_exception
.balign 0x80
curr_el_sp0_irq: // The exception handler for the IRQ exception
                // from the current EL using SP0.
    mov x0, #2
    b handle_generic_exception
.balign 0x80
curr_el_sp0_fiq: // The exception handler for the FIQ exception
                 // from the current EL using SP0.
    mov x0, #3
    b handle_generic_exception
.balign 0x80
curr_el_sp0_serror: // The exception handler for the system error
                    // exception from the current EL using SP0.
    mov x0, #4
    b handle_generic_exception
.balign 0x80
curr_el_spx_sync: // The exception handler for the synchronous
                  // exception from the current EL using the
                  // current SP.
    mov x0, #5
    b handle_synchronous_exception
.balign 0x80
curr_el_spx_irq: // The exception handler for IRQ exception
                 // from the current EL using the c
    mov x0, #6
    b handle_generic_exception
 .balign 0x80
curr_el_spx_fiq: // The exception handler for the FIQ exception
                // from the current EL using the current SP.
    mov x0, #7
    b handle_generic_exception
.balign 0x80
curr_el_spx_serror: // The exception handler for the system error
                     // exception from the current EL using the
                    // current SP.
    mov x0, #8
    b handle_generic_exception
.balign 0x80
lower_el_aarch64_sync: // The exception handler for the synchronous
                        // exception from a lower EL (AArch64).
    mov x0, #9
    b handle_generic_exception
.balign 0x80
lower_el_aarch64_irq: // The exception handler for the IRQ exception
                    // from a lower EL (AArch64).
    mov x0, #10
    b handle_generic_exception
.balign 0x80
lower_el_aarch64_fiq: // The exception handler for the FIQ exception
                        // from a lower EL (AArch64).
    mov x0, #11
    b handle_generic_exception
.balign 0x80
lower_el_aarch64_serror: // The exception handler for the system error
                        // exception from a lower EL(AArch64).
    mov x0, #12
    b handle_generic_exception
.balign 0x80
lower_el_aarch32_sync: // The exception handler for the synchronous
                        // exception from a lower EL(AArch32).
    mov x0, #13
    b handle_generic_exception
.balign 0x80
lower_el_aarch32_irq: // The exception handler for the IRQ exception
                        // from a lower EL (AArch32).
    mov x0, #14
    b handle_generic_exception
.balign 0x80
lower_el_aarch32_fiq: // The exception handler for the FIQ exception
                         // from a lower EL (AArch32).
    mov x0, #15
    b handle_generic_exception
.balign 0x80
lower_el_aarch32_serror: // The exception handler for the system error
                        // exception from a lower EL(AArch32).
    mov x0, #16
    b handle_generic_exception


//FEAT_LPA Not Supported
//FEAT_HAFDBS Implemented
//FEAT_HPDS2 Implemented
//ARM_MMU_TABLE_BASE_LO = Table Base Address
//Store a single descriptor there that covers the first 512MB memory Flat Mapped
// T0Sz = 24 bits (40 bits of physical address) so we start at level 2 descriptor
// Descriptor Attributes for cachable memory
//   
//   ignored[63] == 0
//   PBHA[62:59] == 0 //act as if FEAT_HPDS2 is not implemented for now
//   ignored[58:55] == 0
//   UXN/XN[54] == 0 //executable
//   PXN[53] ==  0 //executable
//   Contiguous[52] == 0 //for 64KB granule indicates that 32 adjacent entries point to a contiguous output range with the same
                         // memory attributes
//   DBM[51] == 0 // dirty bit modifer
//   GP[50] == 0 // non-guarded page
//   output addr[x:16] = 0x0
//   nt[16] = 0 //used for changing block size
//   OA[15:2] == 0 //FEAT_LPA not implemented
//   nG[11] == 0 //non-global page
//   AF[10] = 1 //access allowed
//   SH[9:8] = 2 // outer sharable
//   AP[7:6] = 0 //read/write at EL2, not at EL0
//   NS[5] = 0 //secure
//   attrIndx[4:2] = 1 //arbitrarily choose this to be our cacheable attribute
//   Type[1:0] = 2'b01 (for block descriptor)
//
//
//L2 Table size for 40-bit IA == 2048 entries
//Mark first 1GB (2 entries) as cacheable memory
//Mark the rest as device memory
.global setup_early_mmu_tables
setup_early_mmu_tables:
    mov x0, ARM_MMU_TABLE_BASE_LO
    //defining first 2 blocks of the table as cacheable regular memory
    mov x1, #0x405
    str x1, [x0], #8
    orr x1, x1, 0x20000000
    str x1, [ x0], #8
    //loop to set the rest of the entries as device memory
    mov x3, #2 //loop counter
    _mmu_loop_start: lsl x2, x3, #29 //store base address of next region
    mov x1, #0x401 //base memory attribuets
    orr x1, x1, x2 //full memory descriptor
    str x1, [x0], #8 //update and increment offest address
    add x3, x3, #1
    cmp x3, #2048
    b.ne _mmu_loop_start
    DSB SY
    ret

    _early_mmu_done: ret
.global setup_early_mmu_registers
setup_early_mmu:
    
    //load MMU tables with static addresses determined on link
    mov x0, ARM_MMU_TABLE_BASE_LO
    mov x1, ARM_MMU_TABLE_BASE_HI
    msr TTBR0_EL2, x0 
    msr TTBR1_EL2, x1
    mov x0, 0xFF00 // Define attr1 as cacheable non-transient allocated memory, attr0 as device memory
    msr MAIR_EL2, x0
    mrs x0, TCR_EL2
    BIC x0, x0, #0x1F
    BIC x0, x0, #0x7C000
    orr x0, x0, #0X18 //set T0SZ to 24 (40 bit IA), PARANGE to 40-bits, granule size to 64KB
    mov x1, #0x24
    orr x0, x0, x1, LSL #12
    msr TCR_EL2, x0
    ISB
    //enable the MMU
    mrs x0, SCTLR_EL2
    ORR x0, x0, #1
    ORR x0, x0, #4
    msr SCTLR_EL2, x0
    ISB
    ret
    