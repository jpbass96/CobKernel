CSRCS := $(wildcard *.c)
CPPSRCS := $(wildcard *.cpp)
ASRCS := $(wildcard *.S)
COBJS := $(CSRCS:.c=.o)
CPPOBJS := $(CPPSRCS:.cpp=.o)
AOBJS := $(ASRCS:.S=.o)
AllOBJS := $(COBJS) $(CPPOBJS) $(AOBJS)
LOADADDR = 0x80000
VPATH = src/arch/arm64

COBKERNEL_VERBOSITY ?= VERBOSITY_DEBUG

GCCFLAGS = -mcpu=cortex-a76 -mlittle-endian -Wall -O0 -ffreestanding \
           -nostartfiles -nostdlib -nostdinc -g -I ./include

AFLAGS = -mcpu=cortex-a76 -mlittle-endian  -I ./include -O0 -g

CFLAGS = -mcpu=cortex-a76 -mlittle-endian -Wall -fsigned-char -ffreestanding -g \
         -I ./include -O0 -fno-exceptions  -DPRINTF_LONG_SUPPORT -D$(COBKERNEL_VERBOSITY)

CPPFLAGS = -fno-exceptions -fno-rtti -nostdinc++ -mcpu=cortex-a76 -mlittle-endian -Wall -fsigned-char \
			  -ffreestanding -g -I ./include -O0 -mstrict-align -std=c++14 -Wno-aligned-new \

all: clean new baremetal_2712.img

%.o: %.S
	@echo "as $@"
	@gcc $(AFLAGS) -c $< -o $@

%.o: %.c
	@echo "gcc $@"
	@gcc $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo "g++ $@"
	@g++ $(CPPFLAGS) -c $< -o $@

baremetal_2712.img: $(AllOBJS)
	@echo "============================================================================="
	@echo "Linking..."
	@ld -o baremetal_2712.elf -Map baremetal_2712.map -nostdlib \
		--section-start=.init=$(LOADADDR) --no-warn-rwx-segments \
		-g -T linker.ld $(AllOBJS)
	-objcopy -O binary baremetal_2712.elf baremetal_2712.img

clean:
	/bin/rm -f baremetal_2712.elf *.o *.img baremetal_2712.map > /dev/null 2> /dev/null || true

new:
	/bin/clear
