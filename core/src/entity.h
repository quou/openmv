#pragma once

#include "common.h"
#include "core.h"

#define add_componentv(w_, e_, t_, ...) \
	do { \
		t_ init = (t_) { __VA_ARGS__ }; \
		_add_component((w_), (e_), type_info(t_), &init); \
	} while (0)

#define add_component(w_, e_, t_, i_) \
	do { \
		t_ init = i_; \
		_add_component((w_), (e_), type_info(t_), &init); \
	} while (0)

#define get_component(w_, e_, t_) \
	((t_*)_get_component((w_), (e_), type_info(t_)))

#define remove_component(w_, e_, t_) \
	_remove_component((w_), (e_), type_info(t_))

#define has_component(w_, e_, t_) \
	_has_component((w_), (e_), type_info(t_))

#define view(w_, v_, ...) \
	struct view v_ = new_view((w_), count_va_args(__VA_ARGS__), (struct type_info[]) { __VA_ARGS__ }); \
	view_valid(&(v_)); \
	view_next(&(v_))

#define view_get(v_, t_) \
	((t_*)_view_get((v_), type_info(t_)))

#define set_component_create_func(w_, t_, f_) \
	_set_component_create_func((w_), type_info(t_), (f_))

#define set_component_destroy_func(w_, t_, f_) \
	_set_component_destroy_func((w_), type_info(t_), (f_))

typedef u64 entity;
typedef u32 entity_version;
typedef u32 entity_id;

API extern const entity null_entity;
API extern const entity_id null_entity_id;

API entity_version get_entity_version(entity e);
API entity_id      get_entity_id(entity e);
API entity         make_handle(entity_id id, entity_version v);

struct world;

typedef void (*component_create_func)(struct world* world, entity e, void* component);
typedef void (*component_destroy_func)(struct world* world, entity e, void* component);

API struct world* new_world();
API void free_world(struct world* world);
API entity new_entity(struct world* world);
API void destroy_entity(struct world* world, entity e);
API bool entity_valid(struct world* world, entity e);

API u32 get_component_pool_count(struct world* world);
API u32 get_alive_entity_count(struct world* world);

API void _set_component_create_func(struct world* world, struct type_info type, component_create_func f);
API void _set_component_destroy_func(struct world* world, struct type_info type, component_create_func f);

/* Call via the appropriate macros. */
API void* _add_component(struct world* world,    entity e, struct type_info type, void* init);
API void  _remove_component(struct world* world, entity e, struct type_info type);
API bool  _has_component(struct world* world,    entity e, struct type_info type);
API void* _get_component(struct world* world,    entity e, struct type_info type);

#define view_max 16
struct view {
	u32 to_pool[view_max];
	void* pools[view_max];
	u32 pool_count;
	void* pool;
	u32 idx;
	entity e;

	struct world* world;
};

API struct view new_view(struct world* world, u32 type_count, struct type_info* types);
API bool view_valid(struct view* view);
API void* _view_get(struct view* view, struct type_info type);
API void view_next(struct view* view);

#define entity_buffer_default_alloc 8

/* Only starts heap allocating once the count overflows the
 * initial allocation capacity. This is to try to avoid two
 * heap allocations if they aren't necessary. */
struct entity_buffer {
	entity initial_allocation[entity_buffer_default_alloc];
	entity* heap_allocation;

	entity* elements;
	u32 count;
	u32 capacity;
};

API struct entity_buffer* new_entity_buffer();
API void free_entity_buffer(struct entity_buffer* buf);
API void entity_buffer_push(struct entity_buffer* buf, entity e);
API void entity_buffer_clear(struct entity_buffer* buf, struct world* world);

#define entity_buffer_iter(b_, n_) \
	u32 n_ = 0; n_ < (b_)->count; n_++
