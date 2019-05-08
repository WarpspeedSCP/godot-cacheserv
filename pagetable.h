#ifndef PAGETABLE_H
#define  PAGETABLE_H

#include "core/vector.h"
#include "core/object.h"
#include "core/os/thread.h"
#include "core/os/mutex.h"
#include "core/rid.h"
#include "core/set.h"
#include "core/variant.h"
#include "core/ordered_hash_map.h"

struct Page {

	enum CachePolicy {
		KEEP_FOREVER,
		FIFO,
	};

	uint8_t *memory_region;
	uint64_t data_offset;
	CachePolicy cache_policy;
	uint8_t alloc_step;
	bool recently_used;
	bool used;

	Page() {}

	Page(
		uint8_t *i_memory_region,
		uint64_t i_data_offset,
		CachePolicy i_cache_policy = CachePolicy::FIFO
	) : memory_region(i_memory_region),
		data_offset(i_data_offset),
		cache_policy(i_cache_policy),
		alloc_step(0),
		recently_used(false),
		used(false) {}

};

struct Range {
	size_t start;
	size_t end;
};

struct Region {
	size_t start_page_idx; // First page's index.
	size_t size; // Size in pages.
	size_t prev;
	size_t next; // In case the region is not contiguous.

	Region() {}

	Region(
		size_t i_start_page_idx,
		size_t i_size,
		size_t i_prev,
		size_t i_next
	) : start_page_idx(i_start_page_idx),
		size(i_size),
		prev(i_prev),
		next(i_next) {}

};

struct PageTable {
	Vector<Page> pages;
	OrderedHashMap<size_t, Region> used_regions;
	OrderedHashMap<size_t, Region> free_regions;
	size_t available_space;
	size_t used_space;
	size_t total_space;
	size_t last_alloc_end;
};

#endif // !PAGETABLE_H
