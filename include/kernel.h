#ifndef __KERNEL_H
#define __KERNEL_H

#include <types.h>
#include <ptrace.h>

#define GDT_LEN 16
#define IDT_LEN 255
#define NR_CPUS 8

typedef struct {
    u16_t offset0;
    u16_t selector;
    unsigned char ist;
    unsigned char attr;
    u16_t offset1;
    u32_t offset2;
    u32_t zero;
} __pack idt_entry_t;

extern unsigned long gdt[GDT_LEN];
extern idt_entry_t idt[IDT_LEN];
extern tb_ptr_t gdt_ptr;
extern tb_ptr_t idt_ptr;

extern int _text;
extern int _edata;
extern int _end;

extern void ignore_int();
extern void _ignore_int(struct pt_regs* regs);


#endif
