#include "pagetable.h"

PageTable::~PageTable() {
	if(this->memory_region) {
		memdelete_arr(this->memory_region);
	}
}

void PageTable::create() {
	this->free_regions.clear();
	this->used_regions.clear();
	this->pages.clear();
	this->available_space = CS_CACHE_LEN;
	this->used_space = 0;
	this->last_alloc_end = 0;
	this->total_space = CS_CACHE_LEN;

	if(this->memory_region)
		memdelete_arr(this->memory_region);
	this->memory_region = memnew_arr(uint8_t, CS_CACHE_LEN);

	for (int i = 0; i < 50; ++i) {
		this->pages.push_back(
				Page(
						(this->memory_region + i * CS_PAGE_SIZE),
						CS_MEM_VAL_BAD));
	}



	this->free_regions[0] = Region(0, 50, CS_MEM_VAL_BAD, CS_MEM_VAL_BAD);
}

size_t PageTable::allocate(size_t length) {
	size_t curr_start_idx = 0;
	size_t start_idx = 0;
	size_t data_offset = 0;
	size_t paged_length = length / CS_PAGE_SIZE + (length % CS_PAGE_SIZE == 0 ? 0 : 1);

	if (this->pages.size() < (int)paged_length)
		return CS_MEM_VAL_BAD;

	if (this->free_regions.size() == 1 && this->used_regions.size() == 0) {
		this->free_regions.erase(0);

		prepare_region(curr_start_idx, paged_length, &data_offset);

		this->used_regions[0] = Region(curr_start_idx, paged_length, CS_MEM_VAL_BAD, CS_MEM_VAL_BAD);
		this->free_regions[curr_start_idx + paged_length] = Region(curr_start_idx + paged_length, 50 - paged_length, CS_MEM_VAL_BAD, CS_MEM_VAL_BAD);

		this->last_alloc_end = curr_start_idx + paged_length;

	} else {
		auto next_free_region = this->free_regions.front();
		size_t curr_available_size = next_free_region.get().size;
		size_t prev_region = CS_MEM_VAL_BAD;
		size_t rem_length = paged_length;

		start_idx = curr_start_idx = next_free_region.get().start_page_idx;

		while (true) {
			if (this->free_regions.size() == 0) {
				create();
				return allocate(length);
			}

			if (rem_length > curr_available_size) {
				prepare_region(curr_start_idx, curr_available_size, &data_offset);

				if (prev_region != CS_MEM_VAL_BAD)
					this->used_regions[prev_region].next = curr_start_idx;

				rem_length -= curr_available_size;


				// Erase currently used free region and unlink it from the previous one.
				size_t prev_free = this->free_regions[curr_start_idx].prev;
				if(prev_free != CS_MEM_VAL_BAD)
					this->free_regions[prev_free].next = this->free_regions[curr_start_idx].next;
				this->free_regions.erase(curr_start_idx);

				// Add a new entry to used_regions.
				this->used_regions[curr_start_idx] = Region(curr_start_idx, curr_available_size, prev_region, CS_MEM_VAL_BAD);
				prev_region = curr_start_idx;
				next_free_region = this->free_regions.front();
				curr_available_size = next_free_region.get().size;
				curr_start_idx = next_free_region.get().start_page_idx;
			} else {
				prepare_region(curr_start_idx, rem_length, &data_offset);

				size_t prev_free = this->free_regions[curr_start_idx].prev;
				if (prev_free != CS_MEM_VAL_BAD)
					this->free_regions[prev_free].next = CS_MEM_VAL_BAD;

				if (prev_region != CS_MEM_VAL_BAD)
					this->used_regions[prev_region].next = curr_start_idx;

				this->used_regions[curr_start_idx] = Region(curr_start_idx, rem_length, prev_region, CS_MEM_VAL_BAD);

				// Insert a new free region in the placeof the old one.
				this->free_regions[curr_start_idx + rem_length] = Region(curr_start_idx + rem_length, curr_available_size - rem_length, prev_free, this->free_regions[curr_start_idx].next);
				this->free_regions.erase(curr_start_idx);
				break;
			}
		}
	}

	return start_idx;
}

void PageTable::free(size_t index) {
	Region curr_region = {};
	while (index != CS_MEM_VAL_BAD) {
		curr_region = this->used_regions[index];
		for (size_t i = curr_region.start_page_idx; i < curr_region.size + curr_region.start_page_idx; ++i) {
			memset((void *)(this->pages[i].memory_region), 0, CS_PAGE_SIZE);
			this->pages.ptrw()[i].used = false;
			this->pages.ptrw()[i].data_offset = CS_MEM_VAL_BAD;
		}
		index = curr_region.next;
	}
}

void PageTable::prepare_region(size_t start, size_t size, size_t *data_offset) {
	for (size_t i = start; i < start + size; ++i) {
		this->pages.ptrw()[i].used = true;
		this->pages.ptrw()[i].data_offset = *data_offset;
		*data_offset += CS_PAGE_SIZE;
	}
}
