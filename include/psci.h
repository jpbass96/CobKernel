#ifndef _psci_h
#define _psci_h

u32 get_psci_version();
u32 get_core_state(u8 thread, u8 core, u8 cluster);

#endif