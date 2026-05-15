#ifndef _arm_h
#define _arm_h
#include "types.h"
#define readmsr(id, __val) asm volatile("mrs %0, "#id : "=r" (__val)); 
#define writemsr(id, __val) asm volatile("msr " #id ", %0" : : "r"(__val): "memory")

#define DECLARE_ARM64_SEM(_semname) u64 _semame __attribute__((aligned(8)))
#define INIT_ARM64_SEM(__semname) 
typedef u64 arm64_sem;

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

extern void arm_smccc_smc(unsigned long a0, unsigned long a1, unsigned long a2,
                 unsigned long a3, unsigned long a4, unsigned long a5,
                 unsigned long a6, unsigned long a7, struct arm_smccc_res *res);

extern void arm64_take_semaphore_exclusive(arm64_sem *sem);
extern void arm64_put_semaphore_exclusive(arm64_sem *sem);
extern u32 arm64_init_semaphore(arm64_sem *sem);

//check if semaphore is empty as a non-blocking way to get the current state of the semaphore without 
//enabling the exclusive monitor
inline boolean arm64_semaphore_taken(arm64_sem *sem) {
        return *sem == 1;
}

#endif