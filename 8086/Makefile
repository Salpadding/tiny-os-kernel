CFLAGS := -I../include -fno-builtin -fno-stack-protector \
		  -Os -fomit-frame-pointer \
		  -ffreestanding -nostdinc \
		  -fno-asynchronous-unwind-tables \
		  -fno-pic -fno-dwarf2-cfi-asm -Wall 

ifeq ($(SIM),QEMU)
CFLAGS += -DSIM_QEMU
endif

all: install

%.o: %.c
	$(CC) -S $(CFLAGS) -m16  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

pm.o: pm.c
	$(CC) -S $(CFLAGS) -m32 -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

prink.o: prink.c
	$(CC) -S $(CFLAGS) -m32 -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

boot.bin: boot.o
	ld -o boot.elf -T boot.lds $< 
	nm boot.elf > boot.map
	objcopy -O binary boot.elf $@
	

loader.bin: loader.o pm.o printk.o printk_16.o
	ld -o loader.elf -e _start -T loader.lds $^
	nm loader.elf > loader.map
	objcopy -O binary loader.elf $@

install: boot.bin loader.bin
	dd if=boot.bin of=../disk.img conv=notrunc
	dd if=loader.bin of=../disk.img seek=1 conv=notrunc

dump: 
	objdump -b binary -D -mi8086 $(obj)


qemu: kernel.bin
	make clean
	make install
	# 0x500~0x7c00 之间的内存不会被 bios 复位 适合用来放 bootloader
	qemu-system-x86_64 -smp 1 -m 2048 -boot c -hda ../disk.img -echr 0x14 -nographic

clean:
	rm -rf *.bin *.o *.s
