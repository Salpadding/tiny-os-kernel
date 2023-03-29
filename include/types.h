#ifndef __TYPES_H
#define __TYPES_H

#ifdef __ASM__
#define __AC(x,y) (x)
#else
#define __AC(x,y) ( x##y )
#endif

#define PAGE_OFFSET	__AC(0xffff800000000000,UL)
#define __pa(x) (x - PAGE_OFFSET)
#define PAGE_SIZE __AC(4096,UL)


#define ENTRY(name)		\
.globl	name	\
; name##:


#ifndef __ASM__
typedef unsigned long pud;
typedef unsigned long pmd;
typedef unsigned long pgd;
typedef unsigned long pte;


#endif

#endif
