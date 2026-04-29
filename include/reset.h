
/*
 * Raspberry Pi 5 (BCM2712) watchdog-based full reset.
 *
 * The PM/watchdog block is described in Linux's bcm2712.dtsi as:
 *   pm: watchdog@7d200000 { reg = <0x7d200000 0x308>; ... }
 *
 * On the Pi 5 AArch64 physical map used by other peripherals in this repo,
 * that typically appears at 0x107d200000 (0x10'0000'0000 + 0x7d200000).
 *
 * Register offsets match the long-standing BCM2835 PM watchdog layout used by
 * drivers/watchdog/bcm2835_wdt.c in Linux.
 */

#ifndef _reset_h
#define _reset_h


void pi5_watchdog_full_reset(void);

#endif
