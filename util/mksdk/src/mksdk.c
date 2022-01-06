#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"
#include "core.h"
#include "microtar.h"

#define buffer_size 1048576
u8* buffer;

static const char* files[] = {
	"premake5.lua",
	"core/premake5.lua",
	"bootstrapper/premake5.lua",
	"logic/premake5.lua",
	"res/CourierPrime.ttf",
	"res/shaders/sprite.glsl",
	"lua54/premake5.lua",
	"lua54/LICENSE",
	"util/packer/premake5.lua",
	"util/mksdk/premake5.lua",
	"util/script_tester/premake5.lua",
	null
};

static const char* dirs[] = {
	"core/src/",
	"bootstrapper/src/",
	"util/packer/src/",
	"util/mksdk/src/",
	"util/script_tester/src/",
	"util/script_tester/res/",
	"lua54/src/",
	null
};

bool add_file_ex(mtar_t* tar, const char* src, const char* dst) {
	FILE* in = fopen(src, "rb");

	if (!in) {
		fprintf(stderr, "Failed to open file: %s\n", src);
		return false;
	}

	printf("Adding: %s\tDestination: %s\n", src, dst);

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
	DIR* dir = opendir(dirname);
	if (!dir) {
		fprintf(stderr, "Failed to open directory: %s\n", dirname);
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0)  { continue; }
		if (strcmp(entry->d_name, "..") == 0) { continue; }

		char full[512];
		strcpy(full, dirname);
		if (full[strlen(full) - 1] != '/') {
			strcat(full, "/");
		}
		strcat(full, entry->d_name);

		struct stat s;
		if (stat(full, &s) != -1) {
			if (S_ISREG(s.st_mode)) {
				add_file(tar, full);
			} else if (S_ISDIR(s.st_mode)) {
				add_dir(tar, full);
			}
		}
	}

	closedir(dir);
}

i32 main() {
	buffer = core_alloc(buffer_size);

	mtar_t tar;
	mtar_open(&tar, "sdk.tar", "w");

	add_file_ex(&tar, "util/mksdk/template/logic.c", "logic/src/main.c");
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
