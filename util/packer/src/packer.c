#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "core.h"

#define buffer_size 1000000
u8 buffer[buffer_size];

#define max_files 512
const char* files[max_files];
u32 file_count;

i32 main() {
	FILE* list_f = fopen("packed.include", "r");
	if (!list_f) {
		fprintf(stderr, "Failed to read `packed.include' for reading.\n");
		return 1;
	}

	FILE* out = fopen("res.pck", "wb");
	if (!out) {
		fprintf(stderr, "Failed to open `res.pck' for writing.\n");
		return 1;
	}

	char* line = null;
	u64 len = 0;
	u64 read = 0;

	file_count = 0;

	while ((read = getline(&line, &len, list_f)) != -1) {
		line[strlen(line) - 1] = '\0';

		FILE* file = fopen(line, "r");
		if (!file) {
			fprintf(stderr, "Failed to open `%s' for reading.\n", line);
			continue;
		}

		fclose(file);

		files[file_count++] = copy_string(line);
	}

	u64 header_size = file_count * (sizeof(u64) * 3) + sizeof(u64);
	u64 cur_size = header_size;

	fwrite(&header_size, sizeof(header_size), 1, out);

	for (u32 i = 0; i < file_count; i++) {
		u64 hash = elf_hash(files[i], strlen(files[i]));

		FILE* file = fopen(files[i], "r");

		fseek(file, 0, SEEK_END);

		u64 size = ftell(file);

		fwrite(&hash, sizeof(hash), 1, out);
		fwrite(&cur_size, sizeof(cur_size), 1, out);
		fwrite(&size, sizeof(size), 1, out);

		cur_size += size;

		fclose(file);
	}

	for (u32 i = 0; i < file_count; i++) {
		FILE* file = fopen(files[i], "r");

		fseek(file, 0, SEEK_END);
		u64 file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		for (u64 ii = 0; ii < file_size; ii++) {
			u64 read = fread(buffer, 1, buffer_size, file);
			fwrite(buffer, read, 1, out);
		}

		fclose(file);
	}

	if (line) {
		free(line);
	}

	fclose(out);
	fclose(list_f);

	return 0;
}
