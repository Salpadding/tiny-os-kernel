#ifndef __MM_IMPL_H
#define __MM_IMPL_H
#include <mm.h>

#define page_to_pfn(page) ((unsigned long)((page) - (mem_map)))
#define MAX_ORDER 11
#define MIGRATE_UNMOVABLE     0
#define MIGRATE_RECLAIMABLE   1
#define MIGRATE_MOVABLE       2
#define MIGRATE_PCPTYPES      3 /* the number of types on the pcp lists */
#define MIGRATE_RESERVE       3
#define MIGRATE_ISOLATE       4 /* can't allocate from here */
#define MIGRATE_TYPES         5

struct kmem_cache{};
struct address_space{};

struct free_area {
    struct list_head free_list[5];
    unsigned long nr_free;
};

extern struct free_area free_area[MAX_ORDER];

enum zone_watermarks {
	WMARK_MIN,
	WMARK_LOW,
	WMARK_HIGH,
	NR_WMARK
};

enum pageflags {
	PG_locked,		/* Page is locked. Don't touch. */
	PG_error,
	PG_referenced,
	PG_uptodate,
	PG_dirty,
	PG_lru,
	PG_active,
	PG_slab,
	PG_owner_priv_1,	/* Owner use. If pagecache, fs may use*/
	PG_arch_1,
	PG_reserved,
	PG_private,		/* If pagecache, has fs-private data */
	PG_private_2,		/* If pagecache, has fs aux data */
	PG_writeback,		/* Page is under writeback */
	PG_head,		/* A head page */
	PG_tail,		/* A tail page */
	PG_swapcache,		/* Swap page: swp_entry_t in private */
	PG_mappedtodisk,	/* Has blocks allocated on-disk */
	PG_reclaim,		/* To be reclaimed asap */
	PG_buddy,		/* Page is free, on buddy lists */
	PG_swapbacked,		/* Page is backed by RAM/swap */
	PG_unevictable,		/* Page is "unevictable"  */
	PG_mlocked,		/* Page is vma mlocked */
	PG_uncached,		/* Page has been mapped as uncached */
	__NR_PAGEFLAGS,

	/* Filesystems */
	PG_checked = PG_owner_priv_1,

	/* Two page bits are conscripted by FS-Cache to maintain local caching
	 * state.  These bits are set on pages belonging to the netfs's inodes
	 * when those inodes are being locally cached.
	 */
	PG_fscache = PG_private_2,	/* page backed by cache */

	/* XEN */
	PG_pinned = PG_owner_priv_1,
	PG_savepinned = PG_dirty,

	/* SLOB */
	PG_slob_free = PG_private,

	/* SLUB */
	PG_slub_frozen = PG_active,
	PG_slub_debug = PG_error,
};

#define MAX_NR_ZONES 4


/*
 * Macros to create function definitions for page flags
 */
#define TESTPAGEFLAG(uname, lname)					\
static inline int Page##uname(struct page *page) 			\
			{ return test_page_bit(PG_##lname, page); }


#define CLEARPAGEFLAG(uname, lname)					\
static inline void ClearPage##uname(struct page *page)			\
			{ clear_page_bit(PG_##lname, page); }


#define SETPAGEFLAG(uname, lname)					\
static inline void SetPage##uname(struct page *page)			\
			{ set_page_bit(PG_##lname, page); }


#define PFLAG(uname, lname) TESTPAGEFLAG(uname, lname) \
    CLEARPAGEFLAG(uname, lname) \
    SETPAGEFLAG(uname, lname)


static inline int test_page_bit(unsigned int nr, struct page* page) {
    return (page->flags & (1UL << nr)) >> nr;
}


static inline void clear_page_bit(unsigned int nr, struct page* page) {
    page->flags ^= (1UL << nr);
}


static inline void set_page_bit(unsigned int nr, struct page* page) {
    page->flags |= (1UL << nr);
}

PFLAG(buddy, buddy);
PFLAG(reserved, reserved);


static inline unsigned long
__find_combined_index(unsigned long page_idx, unsigned int order)
{
	return (page_idx & ~(1 << order));
}


static inline struct page *
__page_find_buddy(struct page *page, unsigned long page_idx, unsigned int order)
{
	unsigned long buddy_idx = page_idx ^ (1 << order);

	return page + (buddy_idx - page_idx);
}


static inline int page_is_buddy(struct page* page, struct page* buddy, int order) {
    return Pagebuddy(buddy) && buddy->private == order;
}

static inline void __free_one_page(struct page *page, unsigned int order, int migratetype) {
    unsigned long page_idx; 

    page_idx = page_to_pfn(page) & ((1 << MAX_ORDER) - 1);


    while (order < MAX_ORDER - 1) {
        unsigned long combined_idx;
        struct page* buddy;

        buddy = __page_find_buddy(page, page_idx, order);

        if (!page_is_buddy(page, buddy, order)) {
            break;
        }
        
        list_del(&buddy->lru);
        free_area[order].nr_free--;
        ClearPagebuddy(buddy);
        buddy->private = 0;
        combined_idx = __find_combined_index(page_idx, order);
        page = page + (combined_idx - page_idx);
        page_idx = combined_idx;
        order++;
    }
    // 标记为 buddy page
    page->private = order;
    SetPagebuddy(page);
    list_add(&page->lru, &(free_area[order].free_list[migratetype]));
    free_area[order].nr_free++;
}


#endif
