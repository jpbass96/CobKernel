#include "base.h"
#include "util.h"
#include "printf.h"
#include "rp1_pcie.h"

void pcie_fixups() {

  write32(PCIE_BASE + 0x4008, 0x263480);
  write32(PCIE_BASE + 0x40a4, 0x82000);
  write32(PCIE_BASE + 0x40a8, 0xb2d0000);
  write32(PCIE_BASE + 0x43c, 0x30060400);
  write32(PCIE_BASE + 0x188, 0x0);
  write32(PCIE_BASE + 0xac, 0x420010);

  rp1_pcie_cfg_write(1, 0, 0x10, 0x80410000);
  
}


void uart_fixups() {

  write32(UART_BASE + 0x24, 0x1b);
  write32(UART_BASE + 0x28, 0X8);
  write32(UART_BASE + 0x2c, 0x70);
  write32(UART_BASE + 0x30, 0xf01);


  //UARTIBRD = 0x1b
  //UARTFBRD = 0x8
}
