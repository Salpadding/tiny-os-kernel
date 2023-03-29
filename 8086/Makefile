CFLAGS := -I../include -fno-builtin -fno-stack-protector \
		  -Os -fomit-frame-pointer \
		  -ffreestanding -nostdinc -static \
		  -fno-pic -fno-dwarf2-cfi-asm

%.o: %.c
	$(CC) -S $(CFLAGS) -m16  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

all: install

boot.bin: boot.o
	ld -M -o boot.elf -T boot.lds $< > loader.map
	objcopy -O binary boot.elf $@
	
pm.o: pm.c
	$(CC) -S $(CFLAGS) -m32 -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

loader.bin: loader.o pm.o
	ld -M -o loader.elf -e _start -T loader.lds $^ > loader.map
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
