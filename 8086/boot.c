#include <8086.h>
#include <io.h>
#include <serial.h>
#include <bios_call.h>

struct disk_address_packet dap __align(ALIGN_DESC) = { 
    .dap_size = 0x10,
    .count = 0,
    .address = 0,
    .segment = 0,
    .lba_low = 0,
    .lba_high = 0,
};


// __entry 配合 boot.lds 强制让 _start 位于 .text 的开始
void __entry _start() {
    RESET_SEG(0);
    unsigned long i;

    //  复位栈寄存器
    asm volatile(
        "mov %%ax, %%sp\n\t"
        "mov %%ax, %%bp\n\t"
        ::"a"(0xfff0):
    );

    // 初始化串口
    INIT_SERIAL;

    unsigned long code;

    #define SECS_PER_READ 4
    // 每次读取 512 byte, 重复32次

    puts("\nread boot loader from disk\n");
    for(i = 0; i < LOADER_SECS / SECS_PER_READ; i++) {
        dap.lba_low = i * SECS_PER_READ + 1;
        dap.address = i * SECS_PER_READ * SEC_SIZE + LOADER_ENTRY; 
        dap.count = SECS_PER_READ;
        code = bios_read_secs(&dap);

        if (code < 0) {
            puts("read sectors by bios call failed\n");
            while(1);
        }
    }


jmp_loader:
    puts("\n Starting 8086... jump to loader\n");
    asm volatile("jmp $0, %0" ::"i"(LOADER_ENTRY):);
}
