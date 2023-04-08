#ifndef __TYPES_H
#define __TYPES_H

#ifdef __ASM__
#define __AC(x,y) (x)
#else
#define __AC(x,y) ( x##y )
#endif

#define E820_MAP_ADDR 0x8000
#define E820_MAP_LEN (u16*)(0x7e00)
#define PAGE_OFFSET	__AC(0xffff800000000000,UL)
#define INIT_PAGETABLE ((void*)0x10000)
#define __pa(x) (((unsigned long)x) - PAGE_OFFSET)
#define __va(x) (((unsigned long)x) + PAGE_OFFSET)
#define PAGE_SIZE __AC(4096,UL)
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define NULL ((void*)0)
#define false 0
#define true 1

/*
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x,y) ((__typeof__(x))((y)-1))
#define round_up(x,y) ((((x)-1) | __round_mask(x,y))+1)
#define round_down(x,y) ((x) & ~__round_mask(x,y))

#define RESET_SEG(seg) asm volatile( \
    "mov %%ax, %%ds\n\t" \
    "mov %%ax, %%es\n\t" \
    "mov %%ax, %%fs\n\t" \
    "mov %%ax, %%gs\n\t" \
    "mov %%ax, %%ss\n\t" \
    ::"a"(seg))

#define ENTRY(name)		\
.globl	name	\
; name##:


#define __align(x) __attribute__( ( aligned(x) ) )
#define __pack __attribute__((packed))
#define __noinline __attribute__((noinline))
#define __entry __attribute__((section ("entry")))


#ifndef __ASM__

typedef unsigned short u16_t;
typedef u16_t u16;
typedef unsigned long usize_t;
typedef usize_t usize;

#if __SIZEOF_POINTER__ == 4
    typedef unsigned long u32_t;
    typedef unsigned long long u64_t;
#else
    typedef unsigned int u32_t;
    typedef unsigned long u64_t;
#endif

typedef u32_t u32;
typedef u64_t u64;


typedef u64_t pud;
typedef u64_t pmd;
typedef u64_t pgd;
typedef u64_t pte;

typedef u64_t tb_desc_t;

struct e820_entry {
    u64_t addr;
    u64_t size;
    u32_t type;
} __pack ;

typedef usize_t phys_addr_t;

typedef usize_t bool;
typedef usize_t pgoff_t;


typedef struct {
    u16_t length;
    usize_t addr;
} __pack tb_ptr_t;

typedef struct {
	volatile u16 counter;
} atomic_t;

typedef u16 spinlock_t;

#endif

#endif
