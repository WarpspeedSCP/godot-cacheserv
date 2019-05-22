/*************************************************************************/
/*  pagetable.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef PAGETABLE_H
#define PAGETABLE_H

#include "core/vector.h"
#include "core/object.h"
#include "core/os/thread.h"
#include "core/os/mutex.h"
#include "core/rid.h"
#include "core/set.h"
#include "core/variant.h"
#include "core/ordered_hash_map.h"

#include "cacheserv_defines.h"

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
	uint8_t *memory_region = NULL;
	size_t available_space;
	size_t used_space;
	size_t total_space;
	size_t last_alloc_end;

	void create();
	size_t allocate(size_t length);
	void free(size_t index);
	void prepare_region(size_t start, size_t size, size_t *data_offset);

	~PageTable();
};



#endif // !PAGETABLE_H
