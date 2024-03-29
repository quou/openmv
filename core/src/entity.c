#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "entity.h"

API const entity null_entity = (UINT64_MAX);
API const entity_id null_entity_id = (UINT32_MAX);

entity_version get_entity_version(entity e) {
	return e >> 32;
}

entity_id get_entity_id(entity e) {
	return (entity_id)e;
}

entity make_handle(entity_id id, entity_version v) {
	return ((u64)v << 32) | id;
}

struct pool;

#define free_queue_size 256
#define pool_table_load_factor 0.75

struct pool_table_el {
	u32 key;
	u32 val_idx;
	bool taken;
};

struct world {
	struct pool_table_el* pool_table;
	u32 pool_table_count;
	u32 pool_table_capacity;

	struct pool* pools;
	u32 pool_count;
	u32 pool_capacity;

	entity* entities;
	u32 entity_count;
	u32 entity_capacity;

	u32 alive_entity_count;

	entity_id avail_id;

	i32 iteration_scope;

	void* free_queue[free_queue_size];
	u32 free_queue_count;
};

static void world_push_free(struct world* world, void* ptr);

struct pool {
	i32* sparse;
	u32 sparse_capacity;

	entity* dense;
	u32 dense_count;
	u32 dense_capacity;

	void* data;
	u32 count;
	u32 capacity;

	struct type_info type;

	struct world* world;

	component_create_func on_create;
	component_destroy_func on_destroy;
};

static void init_pool(struct pool* pool, struct world* world, struct type_info t) {
	*pool = (struct pool) { 0 };

	pool->type = t;

	pool->world = world;
}

static void* pool_get(struct pool* pool, entity e);

static void deinit_pool(struct pool* pool) {	
	if (pool->on_destroy) {
		for (u32 i = 0; i < pool->dense_count; i++) {
			const entity e = pool->dense[i];

			void* ptr = pool_get(pool, e);
			pool->on_destroy(pool->world, e, ptr);
		}
	}

	if (pool->sparse) {
		core_free(pool->sparse);
	}
	if (pool->dense) {
		core_free(pool->dense);
	}
	if (pool->data) {
		core_free(pool->data);
	}
}

static bool pool_has(struct pool* pool, entity e) {
	const entity_id id = get_entity_id(e);
	return (id < pool->sparse_capacity) && (pool->sparse[id] != -1);
}

static i32 pool_sparse_idx(struct pool* pool, entity e) {
	return pool->sparse[get_entity_id(e)];
}

static void* pool_add(struct pool* pool, entity e, void* init) {
	if (pool->count >= pool->capacity) {
		u32 capacity = pool->capacity < 8 ? 8 : pool->capacity * 2;
		void* new_allocation = core_alloc(capacity * pool->type.size);
		memcpy(new_allocation, pool->data, pool->capacity * pool->type.size);
		if (pool->data) {
			if (pool->world->iteration_scope <= 0) {
				core_free(pool->data);
			} else {
				world_push_free(pool->world, pool->data);
			}
		}

		pool->data = new_allocation;
		pool->capacity = capacity;
	}

	void* ptr = &((u8*)pool->data)[(pool->count++) * pool->type.size];

	const u32 eid = get_entity_id(e);
	if (eid >= pool->sparse_capacity) {
		const u32 new_cap = eid + 1;
		i32* alloc = core_alloc(new_cap * sizeof(i32));
		memcpy(alloc, pool->sparse, pool->sparse_capacity * sizeof(i32));
		if (pool->sparse) {
			if (pool->world->iteration_scope <= 0) {
				core_free(pool->sparse);
			} else {
				world_push_free(pool->world, pool->sparse);
			}
		}

		pool->sparse = alloc;

		for (u32 i = pool->sparse_capacity; i < new_cap; i++) {
			pool->sparse[i] = -1;
		}
		pool->sparse_capacity = new_cap;
	}

	pool->sparse[eid] = (i32)pool->dense_count;

	if (pool->dense_count >= pool->dense_capacity) {
		u32 dense_capacity = pool->dense_capacity < 8 ? 8 : pool->dense_capacity * 2;
		void* alloc = core_alloc(dense_capacity * sizeof(entity));
		memcpy(alloc, pool->dense, pool->dense_capacity * sizeof(entity));
		if (pool->dense) {
			if (pool->world->iteration_scope <= 0) {
				core_free(pool->dense);
			} else {
				world_push_free(pool->world, pool->dense);
			}
		}

		pool->dense_capacity = dense_capacity;
		pool->dense = alloc;
	}

	pool->dense[pool->dense_count++] = e;

	memcpy(ptr, init, pool->type.size);

	if (pool->on_create) {
		pool->on_create(pool->world, e, ptr);
	}

	return ptr;
}

static void pool_remove(struct pool* pool, entity e) {
	const i32 pos = pool->sparse[get_entity_id(e)];

	if (pool->on_destroy) {
		void* ptr = &((char*)pool->data)[pos * pool->type.size];
		pool->on_destroy(pool->world, e, ptr);
	}

	const entity other = pool->dense[pool->dense_count - 1];

	pool->sparse[get_entity_id(other)] = pos;
	pool->dense[pos] = other;
	pool->sparse[get_entity_id(e)] = -1;

	pool->dense_count--;

	memmove(
		&((char*)pool->data)[pos * pool->type.size],
		&((char*)pool->data)[(pool->count - 1) * pool->type.size],
		pool->type.size);

	pool->count--;
}

static void* pool_get_by_idx(struct pool* pool, i32 idx) {
	return &((char*)pool->data)[idx * pool->type.size];
}

static void* pool_get(struct pool* pool, entity e) {
	return pool_get_by_idx(pool, pool_sparse_idx(pool, e));
}

static void world_push_free(struct world* world, void* ptr) {
	if (world->free_queue_count < free_queue_size) {
		world->free_queue[world->free_queue_count++] = ptr;
		return;
	}

	assert(0 && "Free queue too small.");
}

static void world_clear_free_queue(struct world* world) {
	for (u32 i = 0; i < world->free_queue_count; i++) {
		core_free(world->free_queue[i]);
	}

	world->free_queue_count = 0;
}

static entity generate_entity(struct world* world) {
	if (world->entity_count >= world->entity_capacity) {
		world->entity_capacity = world->entity_capacity < 8 ? 8 : world->entity_capacity * 2;
		world->entities = core_realloc(world->entities, world->entity_capacity * sizeof(entity));
	}

	const entity e = make_handle(world->entity_count, 0);
	world->entities[world->entity_count++] = e;

	return e;
}

static entity recycle_entity(struct world* world) {
	const entity_id cur_id = world->avail_id;
	const entity_version cur_ver = get_entity_version(world->entities[cur_id]);

	world->avail_id = get_entity_id(world->entities[cur_id]);

	const entity recycled = make_handle(cur_id, cur_ver);

	world->entities[cur_id] = recycled;

	return recycled;
}

static void release_entity(struct world* world, entity e, entity_version desired) {
	const entity_id id = get_entity_id(e);
	world->entities[id] = make_handle(world->avail_id, desired);
	world->avail_id = id;
}

static struct pool_table_el* find_pool_el(struct pool_table_el* els, u32 capacity, u32 key) {
	u32 idx = key % capacity;

	for (;;) {
		struct pool_table_el* el = els + idx;
		if (!el->taken || el->key == key) {
			return el;
		}

		idx = (idx + 1) % capacity;
	}
}

static void pool_table_resize(struct world* world, u32 capacity) {
	struct pool_table_el* els = core_calloc(capacity, sizeof(struct pool_table_el));

	for (u32 i = 0; i < world->pool_table_capacity; i++) {
		struct pool_table_el* el = world->pool_table + i;
		if (!el->taken) { continue; }

		struct pool_table_el* dst = find_pool_el(els, capacity, el->key);
		dst->key = el->key;
		dst->val_idx = el->val_idx;
		dst->taken = el->taken;
	}

	if (world->pool_table) { core_free(world->pool_table); }

	world->pool_table = els;
	world->pool_table_capacity = capacity;
}

static struct pool* get_pool(struct world* world, struct type_info type) {
	if (world->pool_table_count == 0) { goto create_new_pool; }

	struct pool_table_el* el = find_pool_el(world->pool_table, world->pool_table_capacity, type.id);
	if (el->taken) { return world->pools + el->val_idx; }

create_new_pool:

	if (world->pool_table_count >= world->pool_table_capacity * pool_table_load_factor) {
		u32 capacity = world->pool_table_capacity < 8 ? 8 : world->pool_table_capacity * 2;
		pool_table_resize(world, capacity);
	}

	el = find_pool_el(world->pool_table, world->pool_table_capacity, type.id);
	el->taken = true;
	el->key = type.id;
	el->val_idx = world->pool_count;

	world->pool_table_count++;

	if (world->pool_count >= world->pool_capacity) {
		u32 pool_capacity = world->pool_capacity < 8 ? 8 : world->pool_capacity * 2;
		void* new_alloc = core_alloc(pool_capacity * sizeof(struct pool));
		memcpy(new_alloc, world->pools, world->pool_capacity * sizeof(struct pool));
		
		if (world->pools) {
			if (world->iteration_scope >= 0) {
				world_push_free(world, world->pools);
			} else {
				core_free(world->pools);
			}
		}

		world->pools = new_alloc;
		world->pool_capacity = pool_capacity;
	}

	struct pool* new = &world->pools[world->pool_count++];
	init_pool(new, world, type);
	return new;
}

static struct pool* get_pool_no_create(struct world* world, struct type_info type) {
	if (world->pool_table_count == 0) { return null; }

	struct pool_table_el* el = find_pool_el(world->pool_table, world->pool_table_capacity, type.id);
	if (el->taken) { return world->pools + el->val_idx; }

	return null;
}

struct world* new_world() {
	struct world* w = core_calloc(1, sizeof(struct world));

	w->avail_id = null_entity_id;

	return w;
}

void free_world(struct world* world) {
	world_clear_free_queue(world);

	if (world->pools) {
		for (u32 i = 0; i < world->pool_count; i++) {
			deinit_pool(&world->pools[i]);
		}

		core_free(world->pools);
	}

	if (world->pool_table) {
		core_free(world->pool_table);
	}

	if (world->entities) {
		core_free(world->entities);
	}

	core_free(world);
}

entity new_entity(struct world* world) {
	world->alive_entity_count++;
	
	if (world->avail_id == null_entity_id) {
		return generate_entity(world);
	} else {
		return recycle_entity(world);
	}
}

void destroy_entity(struct world* world, entity e) {
	for (u32 i = 0; i < world->pool_count; i++) {
		if (pool_has(&world->pools[i], e)) {
			pool_remove(&world->pools[i], e);
		}
	}

	const entity_version nv = get_entity_version(e) + 1;
	release_entity(world, e, nv);

	world->alive_entity_count--;
}

bool entity_valid(struct world* world, entity e) {
	const entity_id id = get_entity_id(e);
	return id < world->entity_count && world->entities[id] == e;
}

u32 get_entity_component_types(struct world* world, entity e, struct type_info* info, u32 count) {
	if (count == 0) { return 0; }

	u32 c = 0;

	for (u32 i = 0; i < world->pool_count; i++) {
		if (pool_has(&world->pools[i], e)) {
			info[c++] = world->pools[i].type;
			if (c > count) {
				return c;
			}
		}
	}

	return c;
}

void _set_component_create_func(struct world* world, struct type_info type, component_create_func f) {
	get_pool(world, type)->on_create = f;
}

void _set_component_destroy_func(struct world* world, struct type_info type, component_create_func f) {
	get_pool(world, type)->on_destroy = f;
}

void* _add_component(struct world* world, entity e, struct type_info type, void* init) {
	struct pool* pool = get_pool(world, type);

	return pool_add(pool, e, init);
}

void _remove_component(struct world* world, entity e, struct type_info type) {
	struct pool* p = get_pool(world, type);
	
	pool_remove(p, e);
}

bool _has_component(struct world* world, entity e, struct type_info type) {
	return pool_has(get_pool(world, type), e);
}

void* _get_component(struct world* world, entity e, struct type_info type) {
	return pool_get(get_pool(world, type), e);
}

u32 get_component_pool_count(struct world* world) {
	return world->pool_count;
}

u32 get_alive_entity_count(struct world* world) {
	return world->alive_entity_count;
}

static bool view_contains(struct view* view, entity e) {
	for (u32 i = 0; i < view->pool_count; i++) {
		if (!pool_has(view->pools[i], e)) {
			return false;
		}
	}

	return true;
}

static u32 view_get_idx(struct view* view, struct type_info type) {
	for (u32 i = 0; i < view->pool_count; i++) {
		if (view->to_pool[i] == type.id) {
			return i;
		}
	}

	return 0;
}

struct view new_view(struct world* world, u32 type_count, struct type_info* types) {
	struct view v = { 0 };
	v.world = world;
	v.pool_count = type_count;

	world->iteration_scope++;

	for (u32 i = 0; i < type_count; i++) {
		v.pools[i] = get_pool_no_create(world, types[i]);
		if (!v.pools[i]) {
			return (struct view) {
				.idx = 0,
				.e = null_entity,
				.world = world
			};
		}

		if (!v.pool) {
			v.pool = v.pools[i];
		} else {
			if (((struct pool*)v.pools[i])->count < ((struct pool*)v.pool)->count) {
				v.pool = v.pools[i];
			}
		}
		v.to_pool[i] = types[i].id;
	}

	if (v.pool && ((struct pool*)v.pool)->count != 0) {
		v.idx = ((struct pool*)v.pool)->count - 1;
		v.e = ((struct pool*)v.pool)->dense[v.idx];
		if (!view_contains(&v, v.e)) {
			view_next(&v);
		}
	} else {
		v.idx = 0;
		v.e = null_entity;
	}

	return v;
}

bool view_valid(struct view* view) {
	bool valid = view->e != null_entity;

	if (!valid) {
		view->world->iteration_scope--;
		
		if (view->world->iteration_scope <= 0) {
			world_clear_free_queue(view->world);
		}
	}

	return valid;
}

void* _view_get(struct view* view, struct type_info type) {
	return pool_get(view->pools[view_get_idx(view, type)], view->e);
}

void view_next(struct view* view) {
	do {
		if (view->idx) {
			view->idx--;
			view->e = ((struct pool*)view->pool)->dense[view->idx];
		} else {
			view->e = null_entity;
		}
	} while ((view->e != null_entity) && !view_contains(view, view->e));
}

struct entity_buffer* new_entity_buffer() {
	struct entity_buffer* buf = core_calloc(1, sizeof(struct entity_buffer));
	buf->capacity = entity_buffer_default_alloc;
	buf->elements = buf->initial_allocation;

	return buf;
}

void free_entity_buffer(struct entity_buffer* buf) {
	if (buf->heap_allocation) {
		core_free(buf->heap_allocation);
	}
	
	*buf = (struct entity_buffer) { 0 };
	buf->capacity = entity_buffer_default_alloc;
	buf->elements = buf->initial_allocation;

	core_free(buf);
}

void entity_buffer_push(struct entity_buffer* buf, entity e) {
	if (buf->count >= buf->capacity) {
		buf->capacity = buf->capacity < entity_buffer_default_alloc ?
			entity_buffer_default_alloc * 2 : buf->capacity * 2;
		buf->heap_allocation = core_realloc(buf->heap_allocation, buf->capacity * sizeof(entity));

		if (buf->elements == buf->initial_allocation) {
			memcpy(buf->heap_allocation, buf->initial_allocation,
				entity_buffer_default_alloc * sizeof(entity));
		}

		buf->elements = buf->heap_allocation;
	}

	buf->elements[buf->count++] = e;
}

void entity_buffer_clear(struct entity_buffer* buf, struct world* world) {
	for (entity_buffer_iter(buf, i)) {
		destroy_entity(world, buf->elements[i]);
	}

	free_entity_buffer(buf);
}
