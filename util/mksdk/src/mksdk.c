#include <string.h>

#include "common.h"
#include "core.h"
#include "microtar.h"
#include "platform.h"

#define buffer_size 1048576
u8* buffer;

static const char* files[] = {
	"core/premake5.lua",
	"bootstrapper/premake5.lua",
	"logic/premake5.lua",
	"res/CourierPrime.ttf",
	"res/DejaVuSans.ttf",
	"res/shaders/sprite.glsl",
	"res/shaders/crt.glsl",
	"res/shaders/invert.glsl",
	"util/packer/premake5.lua",
	"util/packer/lay.out",
	"util/mksdk/premake5.lua",
	"util/test/premake5.lua",
	null
};

static const char* dirs[] = {
	"core/src/",
	"bootstrapper/src/",
	"util/packer/src/",
	"util/mksdk/src/",
	"util/mksdk/template/",
	"util/test/src/",
	"util/test/scripts/",
	null
};

bool add_file_ex(mtar_t* tar, const char* src, const char* dst) {
	FILE* in = fopen(src, "rb");

	if (!in) {
		fprintf(stderr, "Failed to open file: %s\n", src);
		return false;
	}

	printf("%40.40s    =>    %s\n", src, dst);

	fseek(in, 0, SEEK_END);
	u64 size = ftell(in);
	fseek(in, 0, SEEK_SET);

	mtar_write_file_header(tar, dst, size);

	for (u64 i = 0; i < size; i++) {
		u64 read = fread(buffer, 1, buffer_size, in);
		mtar_write_data(tar, buffer, read);
	}

	fclose(in);

	return true;
}

bool add_file(mtar_t* tar, const char* src) {
	return add_file_ex(tar, src, src);
}

void add_dir(mtar_t* tar, const char* dirname) {
	struct dir_iter* it = new_dir_iter(dirname);
	do {
		struct dir_entry* entry = dir_iter_cur(it);

		if (file_is_regular(entry->name)) {
			add_file(tar, entry->name);
		} else if (file_is_dir(entry->name)) {
			add_dir(tar, entry->name);
		}
	} while (dir_iter_next(it));

	free_dir_iter(it);
}

i32 main() {
	buffer = core_alloc(buffer_size);

	mtar_t tar;
	mtar_open(&tar, "sdk.tar", "w");

	add_file_ex(&tar, "util/mksdk/template/logic.c", "logic/src/main.c");
	add_file_ex(&tar, "util/mksdk/template/premake5.lua", "premake5.lua");
	add_file_ex(&tar, "util/mksdk/template/packed.include", "packed.include");

	for (const char** dirname = dirs; *dirname != null; dirname++) {
		add_dir(&tar, *dirname);
	}

	for (const char** file = files; *file != null; file++) {
		add_file(&tar, *file);
	}

	mtar_finalize(&tar);

	mtar_close(&tar);

	core_free(buffer);
}
