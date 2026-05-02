#include "arm.h"
#include "util.h"
#include "types.h"
#include "printf.h"

#define PSCI_VERSION_FUNCID   0x84000000
#define AFFINITY_INFO_FUNCID 0xC4000004

struct __attribute__((packed, aligned(8))) cpu_affinity {
    u8 aff0;
    u8 aff1;
    u8 aff2;
    u8 res0;
    u8 aff3;
    u8 res1[3];
};

union psci_affinity_param {
    struct cpu_affinity affinity;
    u64 raw;
};

u32 get_psci_version() {
    struct arm_smccc_res result;
    
    arm_smccc_smc(PSCI_VERSION_FUNCID, 0, 0, 0, 0, 0, 0, 0, &result);

    
    printf("PSCI Version %x.%x\n\r", get_bits_sz((u32)result.a0,0, 16), get_bits_sz((u32)result.a0,16, 15));

    return result.a0;
}

#ifdef A76
#define CLUSTER_SIZE 4
//A76 single threaded. AFF0 is 0
#define AFF0 0 
//Core in cluster. Can be 0-7
#define AFF1(core) core
//Cluster ID. Should be same as what is in CLUSTERIDAFF2 config signal
#define AFF2(core) (core/CLUSTER_SIZE)
#define AFF3(core) 0
#endif

u32 get_core_state(u8 thread, u8 core, u8 cluster) {
    struct arm_smccc_res result;
    union psci_affinity_param affinity;
    affinity.affinity.aff0 = thread;
    affinity.affinity.aff1 = core;
    affinity.affinity.aff2 = cluster;
    affinity.affinity.aff3 = 0; //what is this one for?
    arm_smccc_smc(AFFINITY_INFO_FUNCID, affinity.raw, 0, 0, 0, 0, 0, 0, &result);
  
    printf("CPU %x:%x ", cluster, core);
    switch(result.a0) {
        case 2:
            printf("ON_PENDING \n\r");
            break;
        case 1:
            printf("OFF \n\r");
            break;
        case 0:
            printf("ON \n\r");
            break;
        default:
            printf("INVALID PSCI COMMAND \n\r");
    }

    return result.a0;

}