CFLAGS := -mcmodel=large -Iinclude -fno-builtin -fno-stack-protector -m64 -Og -g -fomit-frame-pointer -ffreestanding -nostdinc  -Wall

ASFLAGS := --64

objects :=  printk.o init.o entry.o task.o mm.o


ifeq ($(SIM),QEMU)
CFLAGS += -DSIM_QEMU
endif

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.s: %.c
	$(CC) -S $(CFLAGS) -o $@ $<

%.o: %.S
	$(CPP) -D__ASM__ -Iinclude -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

kernel.bin: system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

system:	$(objects)
	ld  -T Kernel.lds  -Bsymbolic -b elf64-x86-64 -nostdlib -static -o system \
		$(objects)


docker:
	docker run -it --rm -v `pwd`:`pwd` --workdir `pwd` centos6 make

install: kernel.bin
	dd if=kernel.bin of=disk.img bs=512 seek=33 conv=notrunc

qemu: 
	make clean -C 8086
	make install SIM=QEMU -C 8086
	make install SIM=QEMU
	# 0x500~0x7c00 之间的内存不会被 bios 复位 适合用来放 bootloader
	qemu-system-x86_64 -smp 1 -m 128 -boot c -hda disk.img -echr 0x14 -nographic \
		-device loader,file=kernel.bin,addr=0x300000,force-raw=on \
		-device loader,file=8086/loader.bin,addr=0x200000,force-raw=on 


bochs: 
	make clean -C 8086
	make clean
	make install -C 8086
	make install
	bochs
clean:
	rm -rf *.o  *.s  system  kernel.bin 


