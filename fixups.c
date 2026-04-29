#include "base.h"
#include "util.h"

void pcie_fixups() {
  write32(PCIE_BASE + 0x400c, 0x0);
  write32(PCIE_BASE + 0x4010, 0x0);
  write32(PCIE_BASE + 0x4070, 0xfff00000);
  write32(PCIE_BASE + 0x4080, 0x1f);
  write32(PCIE_BASE + 0x4084, 0x1f);

  write32(PCIE_BASE + 0x4014, 0x0);
  write32(PCIE_BASE + 0x4018, 0x4);
  write32(PCIE_BASE + 0x4074, 0xfff00000);
  write32(PCIE_BASE + 0x4088, 0x1c);
  write32(PCIE_BASE + 0x408c, 0x1e);

  write32(PCIE_BASE + 0x401c, 0x0);
  write32(PCIE_BASE + 0x4020, 0x0);
  write32(PCIE_BASE + 0x4078, 0x10);
  write32(PCIE_BASE + 0x4090, 0x0);
  write32(PCIE_BASE + 0x4094, 0x0);

  write32(PCIE_BASE + 0x4024, 0x0);
  write32(PCIE_BASE + 0x4028, 0x0);
  write32(PCIE_BASE + 0x407c, 0x10);
  write32(PCIE_BASE + 0x4098, 0x0);
  write32(PCIE_BASE + 0x409c, 0x0);
 
}


void uart_fixups() {

  write32(UART_BASE + 0x24, 0x1b);
  write32(UART_BASE + 0x28, 0X8);
  write32(UART_BASE + 0x2c, 0x70);
  write32(UART_BASE + 0x30, 0xf01);


  //UARTIBRD = 0x1b
  //UARTFBRD = 0x8
}
