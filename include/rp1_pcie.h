#ifndef _rp1_pcie_h
#define _rp1_pcie_h
#include "types.h"

void rp1_read_pcie_windows();
u32 rp1_pcie_cfg_read(u8 bus, u8 devfn, u16 off);
void rp1_pcie_cfg_write(u8 bus, u8 devfn, u16 off, u32 data);



#endif
