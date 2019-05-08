#include "register_types.h"

#include "core/class_db.h"
#include "core/engine.h"
#include "core/project_settings.h"

static FileCacheServer *file_cache_server = NULL;
static _FileCacheServer *_file_cache_server = NULL;
void register_cacheserv_types() {
	file_cache_server = memnew(FileCacheServer);
	file_cache_server->init();
	_file_cache_server = memnew(_FileCacheServer);
	ClassDB::register_class<_FileCacheServer>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("FileCacheServer", _FileCacheServer::get_singleton()));
}

void unregister_cacheserv_types() {
	if (file_cache_server) memdelete(file_cache_server);
	if (_file_cache_server) memdelete(_file_cache_server);
}
