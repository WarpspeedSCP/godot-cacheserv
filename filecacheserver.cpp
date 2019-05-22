#include "filecacheserver.h"

#include <unistd.h>



FileCacheServer::FileCacheServer() {
	this->create_page_table();
	singleton = this;
}

FileCacheServer::~FileCacheServer() {}

FileCacheServer *FileCacheServer::singleton = NULL;
_FileCacheServer *_FileCacheServer::singleton = NULL;

FileCacheServer *FileCacheServer::get_singleton() {
	return singleton;
}

void FileCacheServer::_bind_methods() {}
void _FileCacheServer::_bind_methods() {}

void FileCacheServer::create_page_table() {
	this->page_table.create();
}

// Allocate a potentially non-contiguous memory region of size 'length'.
size_t FileCacheServer::alloc_in_cache(size_t length) {
	return this->page_table.allocate(length);
}

void FileCacheServer::free_regions(size_t idx) {
	this->page_table.free(idx);
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

	size_t curr_region = start_region_idx;
	while(this->page_table.used_regions[curr_region].next != CS_MEM_VAL_BAD)
		curr_region = this->page_table.used_regions[curr_region].next;

	size_t extra_regions = alloc_in_cache(byte_length);
	this->page_table.used_regions[curr_region].next = extra_regions;
	this->page_table.used_regions[extra_regions].prev = curr_region;

	return 0;
}

// Prepares a contiguous run of pages for usage. Updates the data offset by adding the size of the region.
void FileCacheServer::prepare_region(size_t start, size_t size, size_t *data_offset) {
	this->page_table.prepare_region(start, size, data_offset);
}

void FileCacheServer::thread_func(void *p_udata) {
	FileCacheServer *fcs = (FileCacheServer *)p_udata;

	// while (!(fcs->exit_thread)) {
	// 	sleep(10);
	// }

	auto x = fcs->alloc_in_cache(CS_PAGE_SIZE * 2);
	auto y = memnew_arr(uint8_t, CS_PAGE_SIZE * 2);
	memset((void *)y, '!', CS_PAGE_SIZE * 2);

	auto z = fcs->alloc_in_cache(CS_PAGE_SIZE * 3);


	fcs->write_to_regions((void *)y, CS_PAGE_SIZE * 2, x);
	fcs->free_regions(x);
	memdelete_arr(y);

	print_line("It woerks");
}

int FileCacheServer::write_to_regions(void *data, size_t length, size_t start_region) {

	size_t offset = 0;
	Region curr_region = this->page_table.used_regions[start_region];

	while(true) {

		size_t region_size = curr_region.start_page_idx + curr_region.size;

		for (size_t i = curr_region.start_page_idx; i < region_size; ++i) {
			memcpy(this->page_table.pages.ptrw()[i].memory_region, (uint8_t *)data + offset, CS_PAGE_SIZE);
			this->page_table.pages.ptrw()[i].data_offset = offset;
			this->page_table.pages.ptrw()[i].recently_used = true;
			offset += CS_PAGE_SIZE;
		}

		if(curr_region.next != CS_MEM_VAL_BAD)
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
