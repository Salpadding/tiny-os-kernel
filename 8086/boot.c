#include <8086.h>
#include <io.h>
#include <serial.h>
#include <bios_call.h>

struct disk_address_packet dap __align(ALIGN_DESC) = { 
    .magic = 0x10,
    .count = 0,
    .address = 0,
    .page = 0,
    .lba_low = 0,
    .lba_high = 0,
};

// __entry 配合 boot.lds 强制让 _start 位于 .text 的开始
void __entry _start() {
    RESET_SEG(0);

    //  复位栈寄存器
    asm volatile(
        "mov %%ax, %%sp\n\t"
        "mov %%ax, %%bp\n\t"
        ::"a"(BOOT_ENTRY):
    );

    // 初始化串口
    INIT_SERIAL;

    // loader.bin 一般小于 16kb
    unsigned short dst = LOADER_ENTRY;
    unsigned long code;

    // qemu 可以直接把 loader 读到文件里面
    // 没有必要去读磁盘了
    int i;

    // 每次读取 512 byte, 重复32次
    for(i = 0; i < 32; i++) {
        dap.address = dst;
        dap.count = 1;
        dap.lba_low = i + 1;

        code = bios_read_secs(i + 1, &dap);
        dst += 0x200;

        if (code < 0) {
            puts("read sectors by bios call failed\n");
            while(1);
        }
    }

jmp_loader:
    puts("\n Starting 8086... jump to loader\n");
    asm volatile("jmp $0, %0" ::"i"(LOADER_ENTRY):);
}
