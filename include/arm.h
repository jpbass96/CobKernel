#ifndef _arm_h
#define _arm_h

#define readmsr(id, __val) asm volatile("mrs %0, "#id : "=r" (__val)); 

/**
 * struct arm_smccc_res - Result from SMC/HVC call
 * @a0-a3 result values from registers 0 to 3
 */
struct arm_smccc_res {
        unsigned long a0;
        unsigned long a1;
        unsigned long a2;
        unsigned long a3;
};

void arm_smccc_smc(unsigned long a0, unsigned long a1, unsigned long a2,
                 unsigned long a3, unsigned long a4, unsigned long a5,
                 unsigned long a6, unsigned long a7, struct arm_smccc_res *res);

#endif