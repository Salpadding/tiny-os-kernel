#include <8086.h>
#include <io.h>
#include <serial.h>
#include <bios_call.h>


tb_desc_t  gdt[3]  __align(ALIGN_DESC) = {
    [0] = { .lower = 0, .upper = 0 },
    [1] = { .lower = 0x0000FFFF, .upper = 0x00CF9A00},
    [2] = { .lower = 0x0000FFFF, .upper = 0x00CF9200},
};

tb_ptr_t  gdt_ptr __align(ALIGN_DESC) = {
    .length = sizeof(gdt) - 1,
    .addr = &gdt, 
};

// __entry 配合 linker script
// 强制把_start 里面的代码放到程序开头
void __entry _start() {
    cli();

    // 复位栈指针
    asm volatile(
    "movl $0x9000, %eax\n\t"
    "movl %eax, %esp\n\t"
    "movl %eax, %ebp\n\t"
    );

    puts("ready for detect memory map\n");

    int code = 0 ;
    unsigned long ebx = 0;
    void* dst = (void*) E820_MAP_ADDR;
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
