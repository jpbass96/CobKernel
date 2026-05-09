#ifndef _psci_h
#define _psci_h

//PSCI Fucntion IDs
#define PSCI_VERSION_FUNCID   0x84000000
#define AFFINITY_INFO_FUNCID  0xC4000004
#define CPUON_FUNCID          0xC4000003

#define PSCI_STATE_CPU_ON 0x0
#define PSCI_STATE_CPU_OFF 1
#define PSCI_STATE_CPU_ON_PENDING 2

//PSCI return codes
#define PSCI_SUCCES  0
#define PSCI_NOT_SUPPORTED  -1
#define PSCI_INVALID_PARAMETERS  -2
#define PSCI_DENIED  -3
#define PSCI_ALREADY_ON  -4
#define PSCI_ON_PENDING  -5
#define PSCI_INTERNAL_FAILURE  -6
#define PSCI_NOT_PRESENT  -7
#define PSCI_DISABLED  -8
#define PSCI_INVALID_ADDRESS  -9

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

u32 get_psci_version();
s64 get_core_state(u8 thread, u8 core, u8 cluster);
s64 psci_cpu_on(struct cpu_affinity affinity, u64 entry, u64 ctxid);

#endif