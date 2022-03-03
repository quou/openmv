#include <stdio.h>

#include "core.h"
#include "platform.h"

struct logic_store {

};

struct logic_store* logic_store;

EXPORT_SYM u64 C_DECL get_storage_size() {
	return sizeof(struct logic_store);
}

EXPORT_SYM void C_DECL on_reload(void* instance) {
	logic_store = instance;
}

EXPORT_SYM struct window* C_DECL create_window() {
	return new_window(make_v2i(800, 600), "OpenMV", false);
}

EXPORT_SYM void C_DECL on_init() {
	printf("Init\n");
}

EXPORT_SYM void C_DECL on_update(f64 ts) {
	printf("Update. Timestep: %g\n", ts);
}

EXPORT_SYM void C_DECL on_deinit() {
	printf("Deinit\n");
}
