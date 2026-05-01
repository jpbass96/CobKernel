#include "rp1_pcie.h"
#include "base.h"
#include "util.h"
#include "printf.h"
#include "pci_ecam.h"



#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LO                0x400c
#define PCIE_MEM_WIN0_LO(win)   \
                PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LO + ((win) * 8)

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_HI                0x4010
#define PCIE_MEM_WIN0_HI(win)   \
                PCIE_MISC_CPU_2_PCIE_MEM_WIN0_HI + ((win) * 8)

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT                0x4070
#define  PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_LIMIT_MASK    0xfff00000
#define  PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_BASE_MASK     0x0000fff0
#define  CPU_ADDR_LO_NUMBITS 12
#define  CPU_ADDR_HI_NUMBITS 8
#define  CPU_BASE_START 4
#define CPU_HI_BASE_START 0
#define CPU_HI_LIMIT_START 0
#define CPU_LIMIT_START 20
#define PCIE_MEM_WIN0_BASE_LIMIT(win)   \
                PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT + ((win) * 4)

#define NUM_WINDOWS 4

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI                   0x4080
#define  PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI_BASE_MASK        0xff
#define PCIE_MEM_WIN0_BASE_HI(win)      \
                PCIE_MISC_CPU_2_PCIE_MEM_WIN0_BASE_HI + ((win) * 8)

#define PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI                  0x4084
#define  PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI_LIMIT_MASK      0xff
#define PCIE_MEM_WIN0_LIMIT_HI(win)     \
                PCIE_MISC_CPU_2_PCIE_MEM_WIN0_LIMIT_HI + ((win) * 8)

#define BRCM_NUM_PCIE_OUT_WINS          0x4
#define SIZE_1M (0x100000)

#define PCIE_MISC_MISC_CTRL                             0x4008

#define get_bits_sz(data, start, size) ((data >> start) & ((1 << size) - 1))
#define get_bits_stop(data, start, stop) ((data >> start) & ((1 << (stop - start + 1)) - 1))



//#define IDX_ADDR(pcie)                  ((pcie)->cfg->offsets[EXT_CFG_INDEX])

//index register base pcie configuration extended address 
#define EXT_CFG_INDEX         0x9000

//address data register to read pci configuration registers from based
//on value in EXT_CFG_INDEX
#define EXT_CFG_DATA          0x8000


#define PCIE_BASE 0x1000120000ULL

//u32 *pcie_base = PCIE_BASE

u32 rp1_pcie_read32(size_t off) {
  return read32(PCIE_BASE + off);
}

void rp1_pcie_write32(size_t off, u32 data) {
  write32(PCIE_BASE + off, data);
}

size_t rp1_map_pcie_cfg(u8 bus, u8 devfn, u16 off) {
  int idx;
  idx = PCIE_ECAM_OFFSET(bus, devfn, 0);
  //write to idx register to map current B:D.F to configuration data read
  //printf("Writing 0x%x to 0x%x\n\r", idx, EXT_CFG_INDEX);
  rp1_pcie_write32(EXT_CFG_INDEX, idx);
  
  //return memory mapped address for given config register read
  return ( EXT_CFG_DATA + PCIE_ECAM_REG(off));
}

//shamelessly taken from the pcie-brcm driver
 
u32 rp1_pcie_cfg_read(u8 bus, u8 devfn, u16 off) {
  //int idx;
  //volatile u32 *dat;
  u64 addr;
  //if (bus == 0) {
  //  printf("dont currently support reading root port config space \n\r");
  //  return 0xFFFF;
  //}

  //get memory mapped address for configuration read
  addr = rp1_map_pcie_cfg(bus, devfn, off);

  //printf("reading data from 0x%x\n\r", (u32)addr);
  return rp1_pcie_read32(addr);

}


void rp1_pcie_cfg_write(u8 bus, u8 devfn, u16 off, u32 data) {

  u64 addr = rp1_map_pcie_cfg(bus, devfn, off);

  rp1_pcie_write32(addr, data);
}

void rp1_read_pcie_windows() {


  for (int i = 0; i < NUM_WINDOWS; i++) {
    u64 pcie_addr, cpu_addr, cpu_limit;
    u64 pcie_lo, pcie_hi;
    u64 cpu_addr_mb_lo, cpu_addr_mb_hi, cpu_limit_mb_lo, cpu_limit_mb_hi;
    //u32 cpu_lo, cpu_hi;
    
    //u64 size;
    u32 reg;

    pcie_lo = rp1_pcie_read32( PCIE_MEM_WIN0_LO(i));
    pcie_hi = rp1_pcie_read32( PCIE_MEM_WIN0_HI(i));

    pcie_addr = pcie_lo | (pcie_hi << 32);


    reg = rp1_pcie_read32(PCIE_MEM_WIN0_BASE_LIMIT(i));
    cpu_addr_mb_lo = get_bits_sz(reg, CPU_BASE_START, CPU_ADDR_LO_NUMBITS);
    // printf("win0 base lo i 0x%x\n");
    cpu_limit_mb_lo = get_bits_sz(reg, CPU_LIMIT_START, CPU_ADDR_LO_NUMBITS);

    reg = rp1_pcie_read32( PCIE_MEM_WIN0_BASE_HI(i));
    //printf("win0 base high is 0x%x\n", reg);
    cpu_addr_mb_hi = get_bits_sz(reg, CPU_HI_BASE_START, CPU_ADDR_HI_NUMBITS);
    // printf("cpu_adr_mb_hi is %llx\n", cpu_addr_mb_hi);

    reg = rp1_pcie_read32( PCIE_MEM_WIN0_LIMIT_HI(i));
    cpu_limit_mb_hi = get_bits_sz(reg, CPU_HI_LIMIT_START, CPU_ADDR_HI_NUMBITS);

    cpu_addr = (cpu_addr_mb_lo | (cpu_addr_mb_hi << CPU_ADDR_LO_NUMBITS)) * 1024 * 1024;
    cpu_limit = (cpu_limit_mb_lo | (cpu_limit_mb_hi << CPU_ADDR_LO_NUMBITS)) * 1024 * 1024;


    //cpu_lo = cpu_addr & 0xFFFFFFFFFF;
    //cpu_hi = (cpu_addr >> 32) & 0xFFFFFFFF;
    //printf("Window %d, cpu_low %d, cpu_hi %d\n\r", i, cpu_lo, cpu_hi);
    //printf("Window %d, cpu_low %x, cpu_hi %x\n\r", i, cpu_lo, cpu_hi);
    
    printf("Window %d %lx bytes, cpu_addr: %lx -> pcie_addr: %lx\n\r", i, (cpu_limit - cpu_addr), cpu_addr, pcie_addr);
    printf("Window %d %x bytes, cpu_addr: %x -> pcie_addr: %x\n\r", i, (u32)(cpu_limit - cpu_addr),  (u32)cpu_addr,  (u32)pcie_addr);
    printf("Window %d %x bytes_hi, cpu_addr_hi: %x -> pcie_addr_hi: %x\n\r", i,  (u32)((cpu_limit - cpu_addr) >> 32),  (u32)(cpu_addr >> 32),  (u32)(pcie_addr >> 32));

    printf("MISC_CTRL: %x\n\r", rp1_pcie_read32(PCIE_MISC_MISC_CTRL));

  }
}
