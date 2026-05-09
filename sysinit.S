//
// sysinit.S
//

.section .text
.global sysinit
sysinit:
    ldr x9, =vector_table_el2
    msr VBAR_EL2, x9 //setup vector table for EL2
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
