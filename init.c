#include <types.h>
#include <serial.h>
#include <printk.h>
#include <asm.h>
#include <task.h>
#include <kernel.h>

void _ignore_int(struct pt_regs* regs) {

}

unsigned long __align(0x10) gdt[GDT_LEN] = {
        0,
		0x0020980000000000,			/*1	KERNEL	Code	64-bit	Segment	08*/
		0x0000920000000000,			/*2	KERNEL	Data	64-bit	Segment	10*/
		0x0000000000000000,			/*3	USER	Code	32-bit	Segment 18*/
		0x0000000000000000,			/*4	USER	Data	32-bit	Segment 20*/
		0x0020f80000000000,			/*5	USER	Code	64-bit	Segment	28*/
		0x0000f20000000000,			/*6	USER	Data	64-bit	Segment	30*/
		0x00cf9a000000ffff,			/*7	KERNEL	Code	32-bit	Segment	38*/
		0x00cf92000000ffff,			/*8	KERNEL	Data	32-bit	Segment	40*/
};




idt_entry_t __align(0x10) idt[IDT_LEN] = {
    0
};

tb_ptr_t __align(0x10) gdt_ptr = {
    .length = sizeof(gdt) - 1,
    .addr = (u64_t) &gdt,
} ;


tb_ptr_t __align(0x10) idt_ptr = {
    .length = sizeof(idt) - 1,
    .addr = (u64_t) &idt,
} ;


void __entry _start() {
    unsigned long tmp;
    unsigned long i;

#ifdef SIM_QEMU
    char* src = 0x100000;
    char* dst = 0x300000;
    // 内存断言
    for(i = 0; i < (unsigned long)((void*)&_edata - (void*)&_text); i++ ) {
        if(*src != *dst) {
            serial_puts("memory assert failed\n");
        }
        src++;
        dst++;
    }
#endif
    // 加载高地址的 gdt ptr
    asm volatile ("lgdt (%0)" ::"r"(&gdt_ptr):);

    // 重置 cs, rip
    asm volatile(
        "leaq 1f(%%rip), %0\n\t"
        "add %1, %0\n\t"
        "push %2\n\t"
        "push %0\n\t"
        "lretq \n\t"
        "1: \n\t" :"=a"(tmp): "b" (PAGE_OFFSET), "i"(KERNEL_CS)
    );


    // 重置所有段寄存器
    RESET_SEG(KERNEL_DS);

    // 重置栈指针
    asm volatile (
        "mov %0, %%rsp\n\t"
        "mov %%rsp, %%rbp\n\t"
        ::"a"(&init_task_union.stack[STACK_SIZE / sizeof(unsigned long)])
    );

    // 设置 idt
    for(i = 0; i < IDT_LEN; i++) {
        idt[i].offset0 = ((unsigned long)&ignore_int) & 0xffff;
        idt[i].selector = KERNEL_CS;
        idt[i].attr = (1<<7) | 0xe;  // present bit + interrupt gate
        idt[i].offset1 = (((unsigned long)&ignore_int) >> 16) & 0xffff;
        idt[i].offset2 = (((unsigned long)&ignore_int) >> 32) & 0xffffffff;
    }

    asm volatile ("lidt (%0)" ::"r"(&idt_ptr):);

    serial_puts("we are in kernel code now\n");
    printk("_text = %p\n", (void*)&_text);
    printk("_end = %p\n", (void*)&_edata);
    printk("text size = %d\n", (void*)&_edata - (void*)&_text);

    init_mm();

    while(1);
}

