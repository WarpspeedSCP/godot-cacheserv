/*************************************************************************/
/*  filecacheserver.h                                                    */
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

#ifndef FILECACHESERVER_H
#define FILECACHESERVER_H

#include "core/object.h"
#include "core/os/mutex.h"
#include "core/os/thread.h"
#include "core/rid.h"
#include "core/set.h"
#include "core/variant.h"
#include "core/vector.h"

#include "pagetable.h"

class FileCacheServer : public Object {
	GDCLASS(FileCacheServer, Object);

	static FileCacheServer *singleton;
	uint8_t *memory_region;
	bool exit_thread;
	PageTable page_table;
	HashMap<RID, Region> regions;
	Thread *thread;
	Mutex *mutex;

private:
	static void thread_func(void *p_udata);

protected:
	static void _bind_methods();

public:
	FileCacheServer();
	~FileCacheServer();

	FileCacheServer *get_singleton();

	Error init();

	void lock();
	void unlock();

	void create_page_table();
	size_t alloc_in_cache(size_t length);
	size_t extend_alloc_space(size_t region_idx, size_t byte_length);
	int free_regions(size_t idx);

	void alloc_region(size_t start, size_t size, size_t *data_offset);
	int write_to_regions(void *data, size_t lenght, size_t start_region);
};

class _FileCacheServer : public Object {
	GDCLASS(_FileCacheServer, Object);

	friend class FileCacheServer;
	static _FileCacheServer *singleton;

protected:
	static void _bind_methods();

public:
	_FileCacheServer();
	static _FileCacheServer *get_singleton();
};

#endif // FILECACHESERVER_H
