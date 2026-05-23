#include "types.h"
#include "arm.h"
#include "util.h"
#include "printf.h"
#include "arm_mmu.h"

//map of value of PARange field of ID_AA64MMFR0_EL1 to bit count
//See ARMv8 Architecture Reference Manual D13.2.61
const u8 ARM_PA_SIZE[7] = {32, 36, 40, 42, 44, 48, 52};

int vmsav8_64_create_table(u64 base, struct vmsav8_64_mmu_table *mmu_table, u8 exception_level) {
    //TODO: Check table base alignment
    u32 tnsz;
    u64 msr;

    switch(exception_level) {
        case 0: //unsupported
            return -1;
            break;
        case 1: //unsupported
            return -1;
            break;
        case 2:
            readmsr(TCR_EL2, msr);
            tnsz = get_bits_sz(msr, 0, 5);
            break;
        case 3: //unsupported
            return -1;
            break;
    }

    //Set level of table pointed to by TTBRx register
    //See Table D5-16 of ARMv8 Architecture Reference Manual
    mmu_table->start_level = tnsz > 34 ? 3 : tnsz > 21 ? 2: 1;
    mmu_table->table_base =  base;

    //See Table D5-18 of ARMv8 Architecture Reference Manual
    //mmu_table->level_size_bits = {0, 52, 29, 16};

    return 0;
}

//table_base should be the base address for this level of the table as returned by TTBRx_ELy or 
//as returned by a previous table stage
void _vmsav8_64_map_block(u64 table_base, u8 level, u64 virt_addr, u64 phys_addr, u32 attr) {

}

void _vmsav8_64_map_page(struct vmsav8_64_mmu_table *mmu_table, u64 phys_addr, u32 attr) {
    
}
//map 
void vmsav8_64_map_64kb(struct vmsav8_64_mmu_table *mmu_table, u64 virt_addr, u64 phys_addr, u32 attr, u64 size) {
   // u8 max_level;
    for (u8 level = mmu_table->start_level; level < 4; level++) {
        if (size < mmu_table->level_size_bits[level]) {
            //max_level = level;
            break;
        }
    }
}

int vmsav8_64_create_table_entry_64kb(u64 next_entry, u16 flags){
    return -1;
}


void mmu_set_block_entry() {

}

void setup_mmu_table() {
    u64 aa64mfr0;
    u64 tcr_el2;
    readmsr(ID_AA64MMFR0_EL1, aa64mfr0);
    readmsr(TCR_EL2, tcr_el2);

    //set translation granule size
    if (ARM_TGRAM64_BITS(aa64mfr0) == 0) {
        tcr_el2 = set_bits(tcr_el2, 1, 14, 2);
    }
    else {
        printf("unsupported granule size\n\r");
        return;
    }
   
    //set physical address size in IPS to match supported PA size in aa64mmfr0
    printf("TCR_EL2 is 0x%lx\n\r", tcr_el2);
    tcr_el2 = set_bits(tcr_el2, ARM_PA_RANGE_BITS(aa64mfr0), 16, 3);
    printf("TCR_EL2 is 0x%lx\n\r", tcr_el2);

    //set T0SZ to support 48-bit address range
    tcr_el2 = set_bits(tcr_el2, 16, 0, 6);
}