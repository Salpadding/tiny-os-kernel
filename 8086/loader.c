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

// 跨段复制
static void __noinline cp(unsigned long dst, unsigned long src, unsigned long count) {
    asm volatile(
        "pushl %%ds\n\t"
        "pushl %%es\n\t"
        "mov %0, %%es\n\t"
        "mov %1, %%ds\n\t"
        "rep movsb\n\t"
        "popl  %%es\n\t"
        "popl  %%ds\n\t" :: 
        "r"( (dst >> 4) & 0xf000 ), 
        "r" ( (src >> 4) & 0xf000), 
        "c" (count), 
        "S"((unsigned long)src & 0xffff), 
        "D"((unsigned long)dst & 0xffff):
    );
}

tb_desc_t  gdt[3]  __align(ALIGN_DESC) = {
    [0] =  0,
    [1] = 0x00CF9A0000000000 ,
    [2] = 0x00CF920000000000,
};

tb_ptr_t  gdt_ptr __align(ALIGN_DESC) = {
    .length = sizeof(gdt) - 1,
    .addr = &gdt, 
};

#define SECS_PER_READ 4
static char cp_buf[SECS_PER_READ * SEC_SIZE];

// __entry 配合 linker script
// 强制把_start 里面的代码放到程序开头
void __entry _start() {
    cli();
    unsigned i, code = 0 ;

    #define BUF_ADDR (unsigned long)(&cp_buf[0])
    // 先读到 buffer 
    // 再跨段从 buffer copy 到 0x10000
    for(i = 0; i < KERNEL_SECS / SECS_PER_READ; i++) {
        dap.lba_low = i * SECS_PER_READ + 1 + LOADER_SECS;
        dap.address = BUF_ADDR;
        dap.count = SECS_PER_READ;
        code = bios_read_secs(&dap);

        cp(KERNEL_TMP + i * SECS_PER_READ * SEC_SIZE, BUF_ADDR, SECS_PER_READ *SEC_SIZE);

        if (code < 0) {
            puts("read sectors by bios call failed\n");
            while(1);
        }
    }

    puts("ready for detect memory map\n");

    unsigned long ebx = 0;
    void* dst = (void*) E820_MAP_ADDR;
    u16* e820_cnt = E820_MAP_LEN;
    *e820_cnt = 0;
    unsigned long cr0;

    // 查询 e820 map
    while(1) {
        code = e820_call(dst, &ebx);
        dst += 20;

         if (ebx == 0) {
             puts("e820 success\n");
             break;
         }

        if (code < 0) {
             puts("e820 error\n");
             while(1);
        }
        
        (*e820_cnt)++;
    }

    // 开启 a20
    if (inb(0x92) & 2) {
        puts("a20 is already enabled\n");
    } else {
        puts("a20 not enabled, go to enable it\n");
        outb_p(inb(0x92) | 2, 0x92);
    }


    // 加载 gdt
    puts("go on load gdt\n");
    asm volatile ("lgdt %0" ::"m"(*&gdt_ptr):);

    // 开启 cr0 保护位 
    asm volatile ("movl %%cr0, %0" :"=r"(cr0)::);
    cr0 |= 1;
    asm volatile ("movl %0, %%cr0" ::"r"(cr0):);

    puts("ready to enter protected mode\n");
    // ljmp 进入保护模式
    asm volatile ("ljmp %0, %1" ::"i"(__INIT_CS), "i"(&pm_start));
}
