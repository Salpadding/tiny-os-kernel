#ifndef __ASM_H
#define __ASM_H

static void invalidate() {
    unsigned long tmp;
    asm volatile (
        "mov %%cr3, %0\n\t"
        "mov %0, %%cr3\n\t"
        :"=r"(tmp):"0"(tmp):
    );
}


#endif
