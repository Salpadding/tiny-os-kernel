#ifndef __TASK_H
#define __TASK_H

#include <types.h>
#include <list.h>

#define STACK_PAGE_ORDER 3
#define STACK_SIZE (PAGE_SIZE << STACK_PAGE_ORDER)


struct task_struct
{
	struct list_head list;
	volatile long state;
	unsigned long flags;

	struct mm_struct *mm;
	struct thread_struct *thread;

	unsigned long addr_limit;	/*0x0000,0000,0000,0000 - 0x0000,7fff,ffff,ffff user*/
					/*0xffff,8000,0000,0000 - 0xffff,ffff,ffff,ffff kernel*/

	long pid;

	long counter;

	long signal;

	long priority;
};


union task_union
{
	struct task_struct task;
	unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
};

extern union task_union init_task_union; 


#endif
