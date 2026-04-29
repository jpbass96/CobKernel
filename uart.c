#include "base.h"
#include "uart.h"
#include "util.h"
#include "types.h"

#define DAT_REG 0x0
#define UARTFR 0x18
#define TXFF_BIT (1 << 5)
#define TXFE_BIT (1 << 7)
#define RXFE_BIT (1 << 4)


char uart_err;

void _wait_fifo_empty() {
  u32 reg;
  reg = 0;

  while (!(reg & TXFE_BIT)) {
    reg = read32(UART_BASE + UARTFR);
  }
}
//void *p needed to support printf implementation. not used
void _write_char(void *p, char c) {

  //wait for fifo to be mpty
  while (!(read32(UART_BASE + UARTFR) & TXFE_BIT));
  
  write32(UART_BASE + DAT_REG, c);
  
}

char _get_char() {
  u32 reg;
  
  while ((read32(UART_BASE + UARTFR) & RXFE_BIT)) {
    //do nothing
  }
  
  reg = read32(UART_BASE + DAT_REG) & 0xFF;
  if (reg & 0xf00) {
    uart_err = (reg & 0xf) >> 8;
  }
  //printf("got byte\n\r");
  //return data byte
  return reg & 0xff;
}

