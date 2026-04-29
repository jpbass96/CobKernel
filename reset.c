

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

#include "reset.h"
#include "types.h"
#include "base.h"
#include "util.h"

#define PM_RSTC 0x1cu
#define PM_WDOG 0x24u

#define PM_PASSWORD 0x5a000000u

#define PM_WDOG_TIME_SET 0x000fffffu
#define PM_RSTC_WRCFG_CLR 0xffffffcfu
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020u


void pi5_watchdog_full_reset(void)
{
  
  /*
   * Linux's bcm2835_wdt uses a very short timeout for restart:
   *   writel_relaxed(10 | PM_PASSWORD, PM_WDOG);
   */
  write32(BCM2712_PM_BASE + PM_WDOG, PM_PASSWORD | (10u & PM_WDOG_TIME_SET));

  u32 cur = read32(BCM2712_PM_BASE + PM_RSTC);
  u32 val = (cur & PM_RSTC_WRCFG_CLR) | PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET;
  write32(BCM2712_PM_BASE + PM_RSTC, val);

  for (;;) {
    __asm__ volatile("wfi");
  }
}
