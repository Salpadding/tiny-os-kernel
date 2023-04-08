asm(".code32");
#include <8086.h>
#include <io.h>
#include <serial.h>


#define PAGE_TABLE_ENTRYS (PAGE_SIZE / sizeof(u64_t))
#define CPU_ID_FN 0x80000000


static void _pm_start();
static void setup_pae();


tb_desc_t  gdt_64[3]  __align(ALIGN_DESC) = {
    [0] = 0,
    [1] = 0x0020980000000000 ,
    [2] = 0x0000920000000000 ,
};

tb_ptr_t  gdt_64_ptr __align(ALIGN_DESC) = {
    .length = sizeof(gdt_64) - 1,
    .addr = &gdt_64, 
};


static unsigned long cpuid(unsigned long eax, unsigned long *edx) {
    asm volatile(
        "cpuid\n\t" 
        "movl %%edx, %1\n\t"
        :"=a"(eax), "=m"(*edx): "0"(eax):);
    return eax;
}

void  pm_start () {
    RESET_SEG(__INIT_DS);
    _pm_start();
}

static void __noinline _pm_start () {
    puts("we are now in protect mode\n");

    unsigned long i;
    char* src = (void*) LOADER_ENTRY;
    char* dst = (void*) 0x200000;

    printk("%d\n", dst);

#ifdef SIM_QEMU
    // 内存断言
    for(i = 0; i < ((unsigned long)&_text_end) - LOADER_ENTRY; i++ ) {
        if(*src != *dst) {
            puts("memory load failed\n");
            while(1);
        }
        src++;
        dst++;
    }
#endif
    

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

    memcpy((void*)KERNEL_ENTRY, (void*)KERNEL_TMP, KERNEL_SECS * SEC_SIZE);
    // 开启 pae
    setup_pae();


    // 准备跳到 0x100000 执行 64 位代码
    asm volatile (
            "lgdt %0\n\t" 
            "ljmp %1, %2"
            ::"m"(*&gdt_64_ptr), "i"(__INIT_CS), "i"(KERNEL_ENTRY)) ;
}


// identity mapping first 2MB
static void setup_pae() {
    unsigned long i;
    u64_t* pud = INIT_PAGETABLE;
    u64_t* pmd = pud + PAGE_TABLE_ENTRYS;
    u64_t* pgd = pmd + PAGE_TABLE_ENTRYS;
    u64_t* pt0 = pgd + PAGE_TABLE_ENTRYS;
    u64_t* pt1 = pt0 + PAGE_TABLE_ENTRYS;

    // clear all
    for(i = 0; i < PAGE_TABLE_ENTRYS * 4; i++) {
        pud[i] = 0;
    } 
    pud[0] = (unsigned long)pmd | 7;
    pud[256] = pud[0];
    pmd[0] = (unsigned long)pgd | 7;
    pgd[0] = (unsigned long)pt0 | 7;
    pgd[1] = (unsigned long)pt1 | 7;

    for(i = 0; i < PAGE_TABLE_ENTRYS * 2; i++) {
        pt0[i] = (i << 12) | 7;
    }

    unsigned long tmp;
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
        "mov %0, %%cr4\n\t" :"=r"(tmp): "i"(1<<5):
    );

    puts("pae setup succed\n");

    // enable long mode
    asm volatile (
        "rdmsr\n\t" 
        "or %1, %%eax\n\t"
        "wrmsr\n\t"
        ::"c"(0xc0000080), "i"(1<<8):);

    puts("long mode enabled\n");

    // enable paging
    asm volatile(
        "movl %%cr0, %0\n\t"
        "or %1, %0\n\t"
        "movl %0, %%cr0\n\t" :"=r"(tmp) : "i" (1<<31)
    );

    puts("paging enabled, we are now in 32-bit compatibility submode\n");
}
