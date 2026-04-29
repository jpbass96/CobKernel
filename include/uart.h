
#ifndef _uart_h
#define _uart_h
#include "types.h"

void _write_char(void *p, char c);
char _get_char();
void _wait_fifo_empty();

#endif
