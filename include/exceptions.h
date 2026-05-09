#ifndef _exceptions_h
#define _exceptions_h

#include "types.h"

void handle_generic_exception(u64 exception);
void handle_synchronous_exception(u64 exception);

#endif