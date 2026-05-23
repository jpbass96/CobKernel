#ifndef _arm_mmu_h
#define _arm_mmu_h

//parameters for what mmu granule to use. Be consistent
//here to optimize 
#define USE_ARM_MMU_64KB_GRANULE
//#define USE_ARM_MMU_16KB_GRANULE
//#define USE_ARM_MMU_4KB_GRANULE

/*
* ARMv8 Architecture Reference manual Section D5.3.1
*   Level 0 lookup is not supported
* 
*/
#ifdef USE_ARM_MMU_64KB_GRANULE
#define MMU_START_LEVEL 1
#define MMU_END_LEVEL 3
#endif


//Register bit defintions for ID_AA64MMFR0_EL1
//Physical address size
#define ARM_PA_RANGE_BITS(msr) get_bits_sz(msr, 0, 4)
//Support for 4KB, 16KB, and 64KB MMU granules
#define ARM_TGRAN4_BITS(msr)  get_bits_sz(msr, 28, 4)
#define ARM_TGRAM16_BITS(msr) get_bits_sz(msr, 20, 4)
#define ARM_TGRAM64_BITS(msr) get_bits_sz(msr, 24, 4)

//TCR_EL2 bit definitions
//
#define TCR_TG0_BITS(msr) get_bits_sz(msr, 14, 2)

//MMU Table Entries
#define BLOCK_ENTRY_ADDR
#define BLOCK_ENTRY_UPPER_ATTR
#define BLOCK_ENTRY_LOWER_ATTR
#define BLOCK_ENTRY

#define PARANGE_TO_BITS(_rangeval) ARM_PA_SIZE[_rangeval]
extern const u8 ARM_PA_SIZE[7];

struct vmsav8_64_mmu_table {
    u64 table_base;
    u8 start_level;
    u8 level_size_bits[4];
    struct vmsav8_64_mmu_ops *mmu_ops;
};

struct vmsav8_64_mmu_ops {
    int (*mmu_map)(struct vmsav8_64_mmu_table *mmu_table, u64 virt_addr, u64 phys_addr, u32 attr, u64 size);
};

#endif