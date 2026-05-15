#include "arm.h"
#include "util.h"
#include "types.h"
#include "printf.h"
#include "psci.h"
#include "time.h"

u32 get_psci_version() {
    struct arm_smccc_res result;
    
    arm_smccc_smc(PSCI_VERSION_FUNCID, 0, 0, 0, 0, 0, 0, 0, &result);

    printf("PSCI Version %x.%x\n\r", get_bits_sz((u32)result.a0,0, 16), get_bits_sz((u32)result.a0,16, 15));

    return result.a0;
}

s64 get_core_state(u8 thread, u8 core, u8 cluster) {
    s64 ret;
    struct arm_smccc_res result;
    union psci_affinity_param affinity;
    
    affinity.affinity.aff0 = thread;
    affinity.affinity.aff1 = core;
    affinity.affinity.aff2 = cluster;
    affinity.affinity.aff3 = 0; //what is this one for?
    arm_smccc_smc(AFFINITY_INFO_FUNCID, affinity.raw, 0, 0, 0, 0, 0, 0, &result);
    ret = (s64)(result.a0);

    char buf[64];
    sprintf(buf, "CPU %x:%x ", cluster, core);
    switch(ret) {
        case PSCI_STATE_CPU_ON_PENDING:
            sprintf(buf, "%sON_PENDING \n\r", buf);
            break;
        case PSCI_STATE_CPU_OFF:
            sprintf(buf, "%sOFF \n\r", buf);
            break;
        case PSCI_STATE_CPU_ON:
            sprintf(buf, "%sON \n\r");
            break;
        default:
            sprintf(buf, "%sINVALID PSCI COMMAND \n\r", buf);
    }

    LOG_INFO("%s", buf);

    return ret;

}

static void _wait_cpu_on(struct cpu_affinity affinity) {
    while (get_core_state(affinity.aff0, affinity.aff1, affinity.aff2) != PSCI_STATE_CPU_ON) {
        //wait for approximately 1ms
        wait_ms(1);
    }
}

s64 psci_cpu_on(struct cpu_affinity affinity, u64 entry, u64 ctxid) {
    struct arm_smccc_res result;
    s64 ret;

    arm_smccc_smc(CPUON_FUNCID, ((union psci_affinity_param)affinity).raw, entry, ctxid, 0, 0, 0, 0, &result);
    ret = result.a0;

    if (ret == PSCI_SUCCES) {
        return 0;
    }

    else if (ret == PSCI_ON_PENDING) {
        _wait_cpu_on(affinity);
        return 0;
    }

    else {
        return -1;
    }
}