# Cobkernel Project Overview

## Description:

This is a baremetal basic kernel/OS for the raspberry pi 5. The beginnings of this environment were created following the guide written by Matthias Steiner: [Raspberry Pi Baremetal Guide](https://www.satyria.de/arm/index.php?title=English). I recommend checking that out for better step-by-step instructions on bringing up a baremetal environment.

This project currently supports a basic console on the GPIO  14/15 UART. This relies on the fact that the PI boot
firmware has already brought up the PCIe interface. See  [base.h](include/base.h) for notes on how offset addresses are calculated 
for various peripherals in the RP1 I/O controller in a baremetal environment

The console currently is a basic string parser that will execute certain installed functions the user types in. There is no file system or appliation support yet. The current features are implemented:

- Early ARM AARCH64 bootstrap code 
- PL011 UART driver for use with an interactive console (GPIO 14/15). We currently rely
  on the firmware to configure the PL011. We just use its FIFO interfaces for TX/RX.
- Support for basic configuration checking of the broadcom PCIe controller link state, address translation windows,
  and PCI configuration space registers.
- Basic semaphore using the ARM LDAXR and STLXR exclusive monitor instructions
- A simple dynamic memory allocator
- UART based printf support
- A workqueue implementation that allows core 0 to pass tasks to cores 1-3.


## Build and Run Instructions
A basic makefile was created to compile all of the code into a bootable .img file. The Makefile currently assumes all code is in the top-level of the cobKernel repo, and that alls includes are in cobKernel/include. Note that i did not set up a cross-compiler. This toolchain was designed to run natively on the raspberry pi itself. 
Run "make clean" to delete all generate objects
Run "make" to build baremetal_2712.img. Move this to /boot/firmware on the raspberry PI SD card partition 1 and boot it as your kernel image.

Set verbosity in your environment with one of:
```
  export COBKERNEL_VERBOSITY=VERBOSITY_DEBUG
  export COBKERNEL_VERBOSITY=VERBOSITY_INFO
  export COBKERNEL_VERBOSITY=VERBOSITY_ERROR
  export COBKERNEL_VERBOSITY=VERBOSITY_NONE
  ```

In order to launch the kernel, a "tryboot.txt" file in /boot/firmware with the following attributes. For a comprehensive list of
 boot configuration attributes, see the raspberry PI user documentation: [config.txt](https://www.raspberrypi.com/documentation/computers/config_txt.html)

### tryboot.txt
```
#Boot firmware automatically enables the AARCH64 state
arm_64bit=1                

#ARM Trusted Firmware occupies first 512KiB. Kernel must start
#after this. Currently the linker assumes this address
kernel_address=0x80000      

#useful if you want to use the JTAG debug probe for coresight debug
enable_jtag_gpio=1          

#name of the kernel image to boot.
kernel=baremetal_2712.img  

#32-bit pixel width in firmware frame buffer
framebuffer_depth=32        

#This is important! This tells boot firmware not to reset the PCIex4 link
#to the RP1 I/O controller. The GPIO 14/15 UART is controlled by the RP1. This allows
#us to access the UART without managing the PCIe link ourselves.
pciex4_reset=0              

#Tells RPI firmware to bring up the UART on RP1. 
enable_rp1_uart=1           

#Gives extra debug info on the UART during firmware boot
uart_2ndstage=1             
```

### Running
Copy the generated baremetal_2712.img to /boot/firmware. Note that modifying files in /boot/firmware requires sudo privileges. 
After tryboot.txt and the image are both in /boot/firmware, you can reboot the raspbi into your image with `sudo reboot '0 
tryboot'`. This will tell the raspbi boot firmware that on the next boot it should boot from the default bootable SD card 
partition, and use tryboot.txt instead of config.txt. Subsequent resets will go back to using config.txt. This provides a 
convenient mechanism to launch CobKernel while still maintaining access to the native Linux OS for development.

# System Description

## Bootstrap Code
The boot flow starts at the _start: routine in [boot.S](boot.S). This simply loads the stack pointer
and jumps straight to the [sysinit](sysinit.S) routine. This is where the interesting bits are. Currently, sysinit splits into
one of two paths. In both paths the vector table is setup with generic exception handlers, the MMU base table address is set
in TTBR_EL2 register, and the MMU is enabled. \
The first path is entered if the core affinity register is 0 indicating this
is the main boot core. In this path, the MMU table is created/configured such the entire 40-bit address space is flat-mapped
from virtual to physical address. Additionally, the first GiB of memory is marked as cacheable and shareable. The rest of memory
is marked as device memory. After configuring the MMU table, it jumps to the main function in [kernel.c](kernel.c) passing in
the start and end address of the heap memory region.\
The second path is entered if the core affinity register is non-zero indicating this is a secondary core. In this path, MMU  table
setup is skipped, and instead the main function is launched with only a single input parameter which is the context pointer
passed by the PSCI interface. Currently this is used to set up a work queue structure.

## Kernel Code
[kernel.c](kernel.c) Finishes the basic boot process. After main is launched, the boot core will finish system setup and launch
the console. This setup includes checking pcie link status, applying PCIe/UART register fixups, attaching the PL011 character
device to the printf implementation, initializing the kernel memory allocator, and booting up the other 3 cores with an atomic
work queue struct that allows passing them tasks. After initialization is done, the console is launched.

## Console
[console.c](console.c) Provides a basic input interface for the user to enter commands. It is currently using hard-coded
string comparisons and commands implemented directly in console.c. Coming soon is a proper command register function to separate
out the commands from the console implementation itself.

## Work Queue
The work queue is a rudimentary task scheduler that allows core 0 to pass "work" to the other 3 cores. The basic principle is that
on bootup the other cores get passed a semaphore protected work queue that they wait for a job to be posted to. The job is a task
handle that has a single void* ptr containing all necessary information for the task to be executed including any input/output
arguments. There are helper functions to help create a task handle routine based on a regular function call. See [primes.c](primes.c) for an example. This basic boilerplate currently allows registering a handle with up to 4 function arguments that
must all be of the same type, although the type itself can be generic. Anything outside of this context will need a custom task
handle implementation.\
Core 0 sets up the work queue with a function pointer and parameters pointer, and then puts the submission queue semaphore. When
a secondary core takes the submission semaphore, it executes the task defined in the work queue entry, and then puts the 
completion queue semaphore. The completion state of the task is stored in the "result" field of the work queue entry. Any 
other return values will be passed through the implementation specific parameters struct. Core 0 can detect when the task is done
by checking the state of the completion queue semaphore. After the task is done, return values can be retrieved from the 
parameters struct if necessary.