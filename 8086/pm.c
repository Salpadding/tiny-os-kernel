asm(".code32");

#include <8086.h>
#include <io.h>
#include <serial.h>


#define INIT_PAGETABLE ((void*)0x10000)
#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRYS (PAGE_SIZE / sizeof(u64))
#define CPU_ID_FN 0x80000000


static void _pm_start();
static void setup_pae();


tb_desc_t  gdt_64[3]  __align(ALIGN_DESC) = {
    [0] = { .lower = 0, .upper = 0 },
    [1] = { .lower = 0, .upper = 0x00209800},
    [2] = { .lower = 0, .upper = 0x00009200},
};

tb_ptr_t  gdt_64_ptr __align(ALIGN_DESC) = {
    .length = sizeof(gdt_64) - 1,
    .addr = &gdt_64, 
};


static unsigned long cpuid(unsigned long eax, unsigned long *edx) {
    asm volatile(
        "cpuid\n\t" 
        "movl %%edx, %1\n\t"
        :"+a"(eax), "=m"(*edx)::);
    return eax;
}

void  pm_start () {
    RESET_SEG(__INIT_DS);
    _pm_start();
}

static void __noinline _pm_start () {
    puts("we are now in protect mode\n");

    unsigned long eax, edx;

    eax = cpuid(CPU_ID_FN, &edx);

    if (eax < CPU_ID_FN + 1) {
        puts("long mode is not supported\n");
        while(1);
    }

    cpuid(CPU_ID_FN + 1, &edx);
    if ( (edx & (1<<29)) == 0) {
        puts("long mode is not supported\n");
        while(1);
    }

    puts("detect long mode support, go on setup page table\n");

    // 开启 pae
    setup_pae();


    while(1);
}


// identity mapping first 2MB
static void setup_pae() {
    unsigned long i;
    u64* pud = INIT_PAGETABLE;
    u64* pmd = pud + PAGE_TABLE_ENTRYS;
    u64* pgd = pmd + PAGE_TABLE_ENTRYS;
    u64* pt = pgd + PAGE_TABLE_ENTRYS;

    // clear all
    for(i = 0; i < PAGE_TABLE_ENTRYS * 4; i++) {
        pud[i].lower = 0;
        pud[i].upper = 0;
    } 
    pud[0].lower = (unsigned long)pmd | 7;
    pmd[0].lower = (unsigned long)pgd | 7;
    pgd[0].lower = (unsigned long)pt | 7;

    for(i = 0; i < PAGE_TABLE_ENTRYS; i++) {
        pt[i].lower = (i << 12) | 7;
    }

    // cr3
    asm volatile ("movl %0, %%cr3\n\t"
                  ::"r"((unsigned long) pud):);

    // load 64bit gdt
    // asm volatile ("lgdt %0" ::"m"(*&gdt_64_ptr));

    RESET_SEG(__INIT_DS);

    // pae
    asm volatile (
        "mov %%cr4, %0\n\t"
        "or  %1, %0\n\t"
        "mov %0, %%cr4\n\t" :"=a"(i): "i"(1<<5):
    );

    puts("pae setup succed\n");

    // enable long mode
    asm volatile (
        "rdmsr\n\t" 
        "or %1, %%eax\n\t"
        "wrmsr\n\t"
        ::"c"(0xc0000080), "i"(1<<8):);

    puts("long mode enabled\n");

    unsigned long tmp;
    // enable paging
    asm volatile(
        "movl %%cr0, %0\n\t"
        "or %1, %0\n\t"
        "movl %0, %%cr0\n\t" :"+r"(tmp) : "i" (1<<31)
    );

    puts("paging enabled, we are now in 32-bit compatibility submode\n");
}
