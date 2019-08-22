#include "data_helpers.h"

#include "file_cache_manager.h"

DescriptorInfo::DescriptorInfo(FileAccess *fa, page_id new_range, int cache_policy) :
		offset(0), guid_prefix(new_range), cache_policy(cache_policy), valid(true) {
	ERR_FAIL_COND(!fa);
	internal_data_source = fa;
	switch (cache_policy) {
		case _FileCacheManager::KEEP:
			max_pages = CS_KEEP_THRESH_DEFAULT;
			break;
		case _FileCacheManager::LRU:
			max_pages = CS_LRU_THRESH_DEFAULT;
			break;
		case _FileCacheManager::FIFO:
			max_pages = CS_FIFO_THRESH_DEFAULT;
			break;
	}
	total_size = internal_data_source->get_len();
	path = internal_data_source->get_path();
	ready_sem = Semaphore::create();
	dirty_sem = Semaphore::create();
	lock = RWLock::create();
}

Variant DescriptorInfo::to_variant(const FileCacheManager &p) {

	Dictionary d;

	for(int i = 0; i < pages.size(); ++i) {
		d[itoh(pages[i]) + " # " + itoh(p.page_frame_map[pages[i]])] = (p.frames[p.page_frame_map[pages[i]]]->to_variant());
	}

	Dictionary out;
	out["offset"] = Variant(itoh(offset));
	out["total_size"] = Variant(itoh(total_size));
	out["guid_prefix"] = Variant(itoh(guid_prefix));
	out["pages"] = Variant(d);
	out["cache_policy"] = Variant(cache_policy);


	return Variant(out);

}
