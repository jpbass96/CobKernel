#include "kernel_info.h"
#include "types.h"
#include "arm.h"
#include "printf.h"
#include "util.h"
#include "arm_mmu.h"

void print_cpsr(u8 elevel) {
  u64 msr, tmp;

  switch(elevel) {
    case 1:
      printf("Reading SPSR_EL1\n\r");
      readmsr(SPSR_EL1, msr);
      break;
    case 2:
      printf("Reading SPSR_EL2\n\r");
      readmsr(SPSR_EL2, msr);
      break;
    case 3:
      printf("Reading SPSR_EL3\n\r");
      readmsr(SPSR_EL3, msr);
      break;
  }

  tmp = get_bits_sz(msr, 0, 4);
  printf("Processor state: ");
  if (get_bits_sz(msr, 4, 1)) {
    printf("  Aarch32  ");
  }
  else {
    printf("  Aarch64 ");
  }
  switch(tmp) {
    case 0b00000:
      printf("  User State\n\r");
      break;

    case 0b00001:
      printf("  FIQ State\n\r");
      break;
    case 0b00010:
      printf("  IRQ State\n\r");
      break;
    case 0b00011:
      printf("  Supervisor State\n\r");
      break;
    case 0b10110:
      printf("  Monitor State\n\r");
      break;
    case 0b00111:
      printf("  Abort State\n\r");
      break;
    case 0b01010:
      printf("  Hypervisor State\n\r");
      break;
    case 0b01011:
      printf("  Undefined State\n\r");
      break;
    case 0b1111:
      printf("  System State\n\r");
      break;
    default:
      printf("  Unknown state!\n\r");
  }
  
  if (!get_bits_sz(msr, 5, 1)) {
    printf("Core operating in ARM state\n\r");
  }
  else {
    printf("Core operating in Thumb or Jazelle state\n\r");
  }

  printf("FIQ Mask Bit: 0x%x\n\r", get_bits_sz(msr, 6, 1));
  printf("IRQ Mask Bit: 0x%x\n\r", get_bits_sz(msr, 7, 1));
  printf("Asynchronous Abort Mask Bit: 0x%x\n\r", get_bits_sz(msr, 8, 1));
  if (get_bits_sz(msr, 9, 1)) {
    printf("Big Endian Operation\n\r");
  }
  else {
    printf("Litle Endian Operation\n\r");
  }

  printf("\n\r");
  
}

void print_scr_el3() {

    u64 msr;

  
    printf("Reading SCR_EL3\n\r");
    readmsr(SCR_EL3, msr);

    if (get_bits_sz(msr, 18, 1)) {
      printf("Secure EL2 enabled\n\r");
    }

    if (get_bits_sz(msr, 0, 1)) {
      printf("Secure EL0/EL1 enabled\n\r");
    }
}

u64 get_core_affinity() {
  u64 msr;
  
  readmsr(MPIDR_EL1, msr);
  return (msr & 0xFFFFFF) | ((msr >> 8) & 0xFF000000);
}

void get_core_context() {
  u64 msr;

  
  readmsr(MIDR_EL1, msr);
  printf("MIDR val is 0x%lx\n\r", msr);

  readmsr(CurrentEL, msr);
  printf("curentEL val is 0x%lx\n\r", msr);

  //print_cpsr(1);
  //print_cpsr(2);

  //print_scr_el3();
  msr = get_core_affinity();
  printf("Core affinity is 0x%lx\n\r", msr);
  printf("\n\r");

  /*
  check the C bit (bit 2) of SCTLR_EL2. controls cacheability
  check the M bit (bit 0) of SCTLR_EL2. enables MMU
  check TCR_EL2 bits 13:8 for shareability and cacheability
  */
  readmsr(SCTLR_EL2, msr);
  msr = set_bits(msr, 1, 2, 1);
  writemsr(SCTLR_EL2, msr);
  printf("SCTLR_EL2: 0x%lx\n\r", msr);

  readmsr(TCR_EL2, msr);
  printf("TCR_EL2: 0x%lx\n\r", msr);

  readmsr(TTBR0_EL2, msr);
  printf("TTBR0_EL2: 0x%lx\n\r", msr);
  readmsr(TTBR1_EL2, msr);
  printf("TTBR1_EL2: 0x%lx\n\r", msr);

  readmsr(ID_AA64MMFR0_EL1, msr);
  
  u32 bits;
  bits = PARANGE_TO_BITS(get_bits_sz(msr, 0, 4));
  printf("Core supports %d bits of Physical Address\n\r", bits);

  if (bits > 48) {
    printf("FEAT_LPA Supported\n\r");
  }
  else {
    printf("FEAT_LPA Not Supported\n\r");
  }

  readmsr(ID_AA64MMFR1_EL1, msr);
  if (get_bits_sz(msr, 12, 4) != 0) {
    printf("FEAT_HAFDBS Implemented\n\r");
  }
  else {
    printf("FEAT_HAFDBS not Implemented\n\r");
  }

  if (get_bits_sz(msr, 12, 4) == 0) {
    printf("FEAT_HPDS Not Implemented\n\r");
  }
  else {
    if (get_bits_sz(msr, 12, 4) == 1)
      printf("FEAT_HPDS Implemented\n\r");
    else
        printf("FEAT_HPDS2 Implemented\n\r");
  }
}
