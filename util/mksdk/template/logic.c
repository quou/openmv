#include <stdio.h>

#include "core.h"

struct logic_store {

};

struct logic_store* logic_store;

API u64 CALL get_storage_size() {
	return sizeof(struct logic_store);
}

API void CALL on_reload(void* instance) {
	logic_store = instance;
}

API void CALL on_init() {
	printf("Init\n");
}

API void CALL on_update(double ts) {
	printf("Update. Timestep: %g\n", ts);
}

API void CALL on_deinit() {
	printf("Deinit\n");
}
