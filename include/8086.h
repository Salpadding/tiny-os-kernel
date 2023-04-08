#ifndef __8086_H
#define __8086_H

#include <io.h>
#include <types.h>

#define BOOT_ENTRY 0x7c00
#define LOADER_ENTRY 0x500
#define LOADER_SECS 32
#define KERNEL_SECS 896
#define KERNEL_TMP 0x10000
#define KERNEL_ENTRY 0x100000
#define ALIGN_DESC 0x10
#define SEC_SIZE 0x200

#define SERIAL_PORT 0x3f8

#define puts(s) serial_puts(s)

// extern 声明的函数不会被 inline
extern int  printk(const char* fmt, ...);
extern int  printk_16(const char* fmt, ...);

#define __INIT_CS 0x08
#define __INIT_DS 0x10

typedef char* va_list;
#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (void*) - 1) / sizeof (void*)) * sizeof (void*))

#define va_start(AP, LASTARG) 						\
 (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))

#define va_end(va_list)

#define va_arg(AP, TYPE)						\
 (AP += __va_rounded_size (TYPE),					\
  *((TYPE *) (AP - __va_rounded_size (TYPE))))

extern int _text_end;
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


static int strlen(const char * s)
{
    int __res;
    int tmp;
asm volatile ("cld\n\t"
	"repne\n\t"
	"scasb\n\t"
	"notl %0\n\t"
	"decl %0"
	:"=c" (__res), "=D"(tmp): "1" (s),"a" (0),"0" (0xffffffff)
	);
return __res;
}




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

static void __noinline memcpy(void* dst, void* src, unsigned long count) {
    asm volatile (
        "cld\n\t" 
        "rep movsb\n\t" ::"D"(dst), "S"(src), "c"(count)
    );
}

#endif
