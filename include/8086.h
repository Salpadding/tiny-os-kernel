#ifndef __8086_H
#define __8086_H

#include <io.h>

#define BOOT_ENTRY 0x7c00
#define LOADER_ENTRY 0x500
#define E820_MAP_ADDR 0x7e00
#define ALIGN_DESC 0x10

#define SERIAL_PORT 0x3f8

#define puts(s) serial_puts(s)
#define __INIT_CS 0x08
#define __INIT_DS 0x10

static void cli() {
    asm volatile ("cli");
}

// 使用汇编打印到串口
static void __puts(const char*s) {
    asm volatile(
"1:  lodsb\n\t"
"testb %%al, %%al\n\t"
"jz 4f\n\t" 
"    call 2f\n\t" 
"    jmp 1b\n\t" 
"2: "
"    pushw %%ax\n\t" 
"3: " 
"    mov %0, %%dx\n\t" 
"    inb %%dx, %%al\n\t" 
"    testb $0x20, %%al\n\t" 
"    jz 3b\n\t" 
"    popw %%ax\n\t" 
"    mov %1, %%dx\n\t" 
"    outb %%al, %%dx\n\t" 
"    ret\n\t"
"4:  \n\t" 
        ::"i"(SERIAL_PORT+5), "i" (SERIAL_PORT), "S" ((unsigned long) s):
);
}

#define __entry __attribute__((section ("entry")))
#define __align(x) __attribute__( ( aligned(x) ) )
#define __pack __attribute__((packed))
#define __noinline __attribute__((noinline))

typedef struct {
    unsigned long lower;
    unsigned long upper;
} u64;

typedef u64 tb_desc_t;

typedef struct {
    unsigned short length;
    void* addr;
} __pack tb_ptr_t;


extern void pm_start();

static void __noinline lba_read_secs (unsigned long lba, unsigned long count, char *dst) {
    unsigned long i;
    outb_p(count & 0xff, 0x1f2); 
    outb_p(lba & 0xff, 0x1f3); 
    outb_p((lba >> 8) & 0xff, 0x1f4); 
    outb_p((lba >> 16) & 0xff, 0x1f5); 
    outb_p(
        ((lba >> 24) & 0xf) | 0xe0
        , 0x1f6); 

    outb_p(0x20, 0x1f7);

    while((inb(0x1f7) & 0x88) != 8);

    for(i = 0; i < count * 256; i++) {
        asm volatile(
            "inw %%dx, %%ax\n\t"
            "movw %%ax, (%%bx)\n\t"
            ::"b"(dst), "d"(0x1f0)
        );
        dst = dst + 2;
    }
}


#define RESET_SEG(seg) asm volatile( \
    "mov %%ax, %%ds\n\t" \
    "mov %%ax, %%es\n\t" \
    "mov %%ax, %%fs\n\t" \
    "mov %%ax, %%gs\n\t" \
    "mov %%ax, %%ss\n\t" \
    ::"a"(seg))

#endif
