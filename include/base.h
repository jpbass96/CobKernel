//
// base.h
//

#ifndef _base_h
#define _base_h

// The RP1 I/O devices are attached to BAR1 of the RP1 PCIe slave. as per the RP1 datasheet
// Section 2.3.1 "The AP accesses Peripherals and Shared SRAM over PCIe as offsets from the assigned base addresses in BAR1 and
//BAR2 respectively" 

//It appears that any transactions targeting BAR1 will be emitted
//starting at address 0xC040000000. 
//      I.E. RP1 system addr = (TLP Addr - BAR1 Addr) + 0xC040000000
//The PCIe TLP address is configured through PCIe controller memory windows
//translating CPU address to PCIe address. From RP1 Data Sheet:
//From reading hardware after RPI firmware and ATF boot the following windows are seen:
//
// Window 0 3ff00000 bytes, cpu_addr: 1c00000000 -> pcie_addr: 80000000 # Points to BAR1
// Window 1 fffffffffff00000 bytes, cpu_addr: 100000 -> pcie_addr: 0 #unconfigured
// Window 2 fffffffffff00000 bytes, cpu_addr: 100000 -> pcie_addr: 0 #unconfigured
// Window 3 fffffffffff00000 bytes, cpu_addr: 100000 -> pcie_addr: 0 #unconfigured
// BAR0 PCI Cfg Addr 0x10: 0x80410000 #RP1 Configuration
// BAR1 PCI Cfg Addr 0x14: 0x80000000 #I/O Devices
// BAR2 PCI Cfg Addr 0x18: 0x80400000 #Shared SRAM
//
//Only window 0 is configured. It is configured such that CPU address 0x1c00000000
//Points directly to the address configured in BAR1. This means that CPU addr
//0x1c00000000 maps to 0xC040000000 in the RP1 Peripheral System Address map.
#define UART_BASE 0x0000001c00030000ULL

//from bcm2712.dtsi
//axi: axi {
//		compatible = "simple-bus";
//		#address-cells = <2>;
//		#size-cells = <2>;
//
//		ranges = <0x00 0x00000000  0x00 0x00000000  0x10 0x00000000>,
//			 <0x10 0x00000000  0x10 0x00000000  0x01 0x00000000>,
//			 <0x14 0x00000000  0x14 0x00000000  0x04 0x00000000>,
//			 <0x18 0x00000000  0x18 0x00000000  0x04 0x00000000>,
//			 <0x1c 0x00000000  0x1c 0x00000000  0x04 0x00000000>;
//pcie2: pcie@1000120000 {
//			compatible = "brcm,bcm2712-pcie";
//			reg = <0x10 0x00120000 0x00 0x9310>;
//AXI bus is flat-mapped from physical addr to AXI addr for
//the ranges listed above. PCIe shows up at 0x1000120000 on the AXI bus.
//PCIe2 falls in the <0x10 0x00000000  0x10 0x00000000  0x01 0x00000000> range
#define PCIE_BASE  0x1000120000ULL

//from bcm2712.dtsi
/// {
//	compatible = "brcm,bcm2712";
//
//	#address-cells = <2>;
//	#size-cells = <2>; 
//soc:  soc@107c000000 {
//		compatible = "simple-bus";
//               CHILD_ADDR     PARENT HIGH, LOW         SIZE
//		ranges = <0x00000000           0x10 0x00000000  0x80000000>;
//      #address-cells = <1>;
//		#size-cells = <1>;
//Physical (parent) address 0x1000000000 maps to soc bus address 0x00000000. Note that the soc
//bus has only one address cell, but the root bus has 2 address cells.
#define RPI5_SOC_BASE 0x1000000000ULL

//from bcm2712-ds.dtsi
//pm: watchdog@7d200000 {
//		compatible = "brcm,bcm2712-pm";
//		reg = <0x7d200000 0x308>;
//PM is at base address 0x7d200000 on the SOC bus
#define BCM2712_PM_BASE (RPI5_SOC_BASE + 0x7d200000ULL)

//from bcm2712-ds.dtsi
//avs_monitor: avs-monitor@7d542000 {
//		compatible = "brcm,bcm2711-avs-monitor",
//					"syscon", "simple-mfd";
//		reg = <0x7d542000 0xf00>;
//AVS is at base address 0x7d542000 on the SOC bus
#define AVS_TEMP_BASE (RPI5_SOC_BASE + 0x107d542000ULL)

//from bcm2712.dtsi
//gio_aon: gpio@7d517c00 {
//			compatible = "brcm,bcm7445-gpio", "brcm,brcmstb-gpio";
//			reg = <0x7d517c00 0x40>;
#define ARM_GPIO2_BASE (RPI5_SOC_BASE + 0x7d517c00)

#define ARM_GPIO2_DATA0 ARM_GPIO2_BASE + 0x04

#endif
