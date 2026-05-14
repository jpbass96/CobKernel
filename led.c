//
// led.c
//

#include "base.h"
#include "util.h"
#include "types.h"
#include "time.h"

void LED_off(void)
{
    u32 reg = read32(ARM_GPIO2_DATA0);
    reg &= ~0x200; // Set bit 9 to 0
    write32(ARM_GPIO2_DATA0, reg);
}

void LED_on(void)
{
    u32 reg = read32(ARM_GPIO2_DATA0);
    reg |= 0x200; // Set bit 9 to 1
    write32(ARM_GPIO2_DATA0, reg);
}

void LED_pulse() {
  LED_off();
  wait_us(500);
  LED_on();
}
