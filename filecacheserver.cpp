#include "filecacheserver.h"

#include <unistd.h>

#define CACHE_LEN (size_t)(4096 * 16)
#define MEM_VAL_BAD (size_t) ~0

FileCacheServer::FileCacheServer() {
	this->memory_region = memnew_arr(uint8_t, CACHE_LEN);
	this->create_page_table();
	singleton = this;
}

FileCacheServer::~FileCacheServer() {
	memdelete(this->memory_region);
}

FileCacheServer *FileCacheServer::singleton = NULL;
_FileCacheServer *_FileCacheServer::singleton = NULL;

FileCacheServer *FileCacheServer::get_singleton() {
	return singleton;
}

void FileCacheServer::_bind_methods() {}
void _FileCacheServer::_bind_methods() {}

void FileCacheServer::create_page_table() {

	this->page_table.free_regions.clear();
	this->page_table.used_regions.clear();
	this->page_table.pages.clear();
	this->page_table.available_space = CACHE_LEN;
	this->page_table.used_space = 0;
	this->page_table.last_alloc_end = 0;
	this->page_table.total_space = CACHE_LEN;

	for (int i = 0; i < 50; ++i) {
		this->page_table.pages.push_back(
				Page(
						(this->memory_region + i * 4096),
						MEM_VAL_BAD));
	}



	this->page_table.free_regions[0] = Region(0, 50, MEM_VAL_BAD, MEM_VAL_BAD);
}

size_t FileCacheServer::alloc_in_cache(size_t length) {

	PageTable &page_table = this->page_table;
	size_t curr_start_idx = 0, start_idx = 0, data_offset = 0;

	size_t paged_length = length / 4096 + (length % 4096 == 0 ? 0 : 1);

	if (page_table.pages.size() < (int)paged_length)
		return MEM_VAL_BAD;

	if (page_table.free_regions.size() == 1 && page_table.used_regions.size() == 0) {
		page_table.free_regions.erase(0);

		alloc_region(curr_start_idx, paged_length, &data_offset);

		page_table.used_regions[0] = Region(curr_start_idx, paged_length, MEM_VAL_BAD, MEM_VAL_BAD);
		page_table.free_regions[curr_start_idx + paged_length] = Region(curr_start_idx + paged_length, 50 - paged_length, MEM_VAL_BAD, MEM_VAL_BAD);

		page_table.last_alloc_end = curr_start_idx + paged_length;

	} else {
		auto next_free_region = page_table.free_regions.front();
		size_t curr_available_size = next_free_region.get().size;
		size_t prev_region = MEM_VAL_BAD;
		size_t rem_length = paged_length;

		start_idx = curr_start_idx = next_free_region.get().start_page_idx;

		while (true) {
			if (page_table.free_regions.size() == 0) {
				create_page_table();
				return alloc_in_cache(length);
			}

			if (rem_length > curr_available_size) {
				alloc_region(curr_start_idx, curr_available_size, &data_offset);

				if (prev_region != MEM_VAL_BAD)
					page_table.used_regions[prev_region].next = curr_start_idx;

				rem_length -= curr_available_size;

				// Erase currently used free region and unlink it from the previous one.
				size_t prev_free = page_table.free_regions[curr_start_idx].prev;
				page_table.free_regions[prev_free].next = MEM_VAL_BAD;
				page_table.free_regions.erase(curr_start_idx);

				// Add a new entry to used_regions.
				page_table.used_regions[curr_start_idx] = Region(curr_start_idx, curr_available_size, prev_region, MEM_VAL_BAD);
				prev_region = curr_start_idx;
				next_free_region = page_table.free_regions.front();
				curr_available_size = next_free_region.get().size;
				curr_start_idx = next_free_region.get().start_page_idx;
			} else {
				alloc_region(curr_start_idx, rem_length, &data_offset);

				size_t prev_free = page_table.free_regions[curr_start_idx].prev;
				if (prev_free != MEM_VAL_BAD)
					page_table.free_regions[prev_free].next = MEM_VAL_BAD;

				if (prev_region != MEM_VAL_BAD)
					page_table.used_regions[prev_region].next = curr_start_idx;

				page_table.used_regions[curr_start_idx] = Region(curr_start_idx, rem_length, prev_region, MEM_VAL_BAD);

				// Insert a new free region in the placeof the old one.
				page_table.free_regions[curr_start_idx + rem_length] = Region(curr_start_idx + rem_length, curr_available_size - rem_length, prev_free, page_table.free_regions[curr_start_idx].next);
				page_table.free_regions.erase(curr_start_idx);
				break;
			}
		}
	}

	return start_idx;
}

int FileCacheServer::free_regions(size_t idx) {
	size_t next_idx = idx;
	Region curr_region;
	while (next_idx != MEM_VAL_BAD) {
		curr_region = this->page_table.used_regions[next_idx];
		for (size_t i = curr_region.start_page_idx; i < curr_region.size + curr_region.start_page_idx; ++i) {
			memset((void *)(this->page_table.pages[i].memory_region), 0, 4096);
			this->page_table.pages.ptrw()[i].used = false;
			this->page_table.pages.ptrw()[i].data_offset = MEM_VAL_BAD;
		}
		next_idx = curr_region.next;
	}
	return 0;
}

void FileCacheServer::unlock() {
	if (!thread || !mutex) {
		return;
	}

	mutex->unlock();
}

void FileCacheServer::lock() {
	if (!thread || !mutex) {
		return;
	}

	mutex->lock();
}

Error FileCacheServer::init() {
	exit_thread = false;
	mutex = Mutex::create();
	thread = Thread::create(FileCacheServer::thread_func, this);
	return OK;
}

// Extend an allocation by byte_length bytes.
size_t FileCacheServer::extend_alloc_space(size_t start_region_idx, size_t byte_length) {

	return 0;
}

// Preps a contiguous run of pages for usage.
void FileCacheServer::alloc_region(size_t start, size_t size, size_t *data_offset) {
	for (size_t i = start; i < start + size; ++i) {
		this->page_table.pages.ptrw()[i].used = true;
		this->page_table.pages.ptrw()[i].data_offset = *data_offset;
		*data_offset += 4096;
	}
}

void FileCacheServer::thread_func(void *p_udata) {
	FileCacheServer *fcs = (FileCacheServer *)p_udata;

	// while (!(fcs->exit_thread)) {
	// 	sleep(10);
	// }

	auto x = fcs->alloc_in_cache(4096 * 2);
	auto y = memnew_arr(uint8_t, 4096 * 2);
	memset((void *)y, '!', 4096 * 2);

	fcs->write_to_regions((void *)y, 4096 * 2, x);
	fcs->free_regions(x);
	memdelete_arr(y);
}

int FileCacheServer::write_to_regions(void *data, size_t length, size_t start_region) {

	size_t offset = 0;
	Region curr_region = this->page_table.used_regions[start_region];

	while(true) {

		size_t region_size = curr_region.start_page_idx + curr_region.size;

		for (size_t i = curr_region.start_page_idx; i < region_size; ++i) {
			memcpy(this->page_table.pages.ptrw()[i].memory_region, (uint8_t *)data + offset, 4096);
			this->page_table.pages.ptrw()[i].data_offset = offset;
			this->page_table.pages.ptrw()[i].recently_used = true;
			offset += 4096;
		}

		if(curr_region.next != MEM_VAL_BAD)
			curr_region = this->page_table.used_regions[curr_region.next];
		else break;
	}
	return 0;
}



_FileCacheServer::_FileCacheServer() {
	singleton = this;
}

_FileCacheServer *_FileCacheServer::get_singleton() {
	return singleton;
}
