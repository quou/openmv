#include <stdio.h>

#include "core.h"
#include "platform.h"

struct logic_store {

};

struct logic_store* logic_store;

API u64 CDECL get_storage_size() {
	return sizeof(struct logic_store);
}

API void CDECL on_reload(void* instance) {
	logic_store = instance;
}

API struct window* CDECL create_window() {
	return new_window(make_v2i(800, 600), "OpenMV", false);
}

API void CDECL on_init() {
	printf("Init\n");
}

API void CDECL on_update(double ts) {
	printf("Update. Timestep: %g\n", ts);
}

API void CDECL on_deinit() {
	printf("Deinit\n");
}
