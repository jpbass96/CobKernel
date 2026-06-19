#include "device.h"
#include "types.h"
#include "base.h"
#include "usb_controller.h"


struct device *usb0;
struct device *usb1;

void init_devices() {
    usb0 = xhci_probe((void*)RP1_USB0_BASE);
    //usb1 = xhci_probe((void*)RP1_USB1_BASE);
}