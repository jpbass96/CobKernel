# Initial code taken from tutorial on www.satyria.de
# Has since been modified
INCLUDE := ./include
include Makefile.global

all: clean new baremetal_2712.img

baremetal_2712.img: $(AllOBJS)
	@$(MAKE) -C src/usb
	@$(MAKE) -C src/kernel
	@mv *.o ./build
	@echo "============================================================================="
	@echo "Linking..."
	@ld -o ./build/baremetal_2712.elf -Map ./build/baremetal_2712.map -nostdlib \
		--section-start=.init=$(LOADADDR) --no-warn-rwx-segments \
		-g -T linker.ld ./build/*.o
	objcopy -O binary ./build/baremetal_2712.elf ./build/baremetal_2712.img

clean:
	/bin/rm -f ./build/baremetal_2712.elf ./build/*.o ./build/*.img ./build/baremetal_2712.map > /dev/null 2> /dev/null || true

new:
	/bin/clear
