#include <types.h>
#include <mm.h>
#include <mm_impl.h>
#include <kernel.h>
#include <printk.h>

struct free_area free_area[MAX_ORDER];

unsigned long max_pfn = 0;
struct page* mem_map;


void init_mm() {
    unsigned long i;
    unsigned long j;
    unsigned long end;

    struct e820_entry* entry = (void*)__va(E820_MAP_ADDR);
     
    for( i = 0; i < *(E820_MAP_LEN); i++) {
        if (entry[i].type != 1) {
            continue;
        }
        end = round_down(entry[i].addr + entry[i].size, PAGE_SIZE);

        if (end/PAGE_SIZE > max_pfn) {
            max_pfn = end / PAGE_SIZE;
        }
        printk("i = %d addr = %x size = %x\n", i, entry[i].addr, entry[i].size) ;
    }

    printk("max_pfn = %d\n", max_pfn);
    printk("sizeof page = %d\n", sizeof(struct page));
    // 计算 memmap pages
    unsigned long pages = round_up(max_pfn * sizeof(struct page), PAGE_SIZE) / PAGE_SIZE;

    printk("pages = %d\n", pages);
   
    mem_map = (void*)round_up((unsigned long)&_end, PAGE_SIZE);
    unsigned long* tmp = (void*)mem_map;

    // 初始化内存为0
    for(i = 0; i < pages * PAGE_SIZE / sizeof(unsigned long); i++) {
        tmp[i] = 0;
    }

    // 初始化双向链表
    for(i = 0; i < pages; i++) {
        INIT_LIST_HEAD(&mem_map[i].lru);
    }
    for(i = 0; i < MAX_ORDER; i++) {
        for(j = 0; j < MIGRATE_TYPES; j++) {
            INIT_LIST_HEAD(&(free_area[i].free_list[j]));
        }
    }

    printk("mem_map = %x\n", mem_map);
    #define MMAP_PFN ((unsigned long)__pa(mem_map))/PAGE_SIZE
    // 把1M以下地址,代码段还有位图占用的页面标记为保留
    for(i = 0; i < MMAP_PFN + pages; i ++) {
        SetPagereserved(&mem_map[i]);
    }
    
    for(i = MMAP_PFN + pages; i < max_pfn; i++) {
        __free_one_page(&mem_map[i], 0, MIGRATE_UNMOVABLE);
    }

    // 至此已完成伙伴系统初始化

    struct page page;

    set_page_bit(0, &page);

    printk("flags = %x\n", page.flags);

    clear_page_bit(0, &page);
    printk("flags = %x\n", page.flags);

    for(i = 0; i < MAX_ORDER; i++) {
        printk("order = %d, count = %d\n", i, free_area[i].nr_free);
    }
}
