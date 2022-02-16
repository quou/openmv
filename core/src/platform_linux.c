#include <time.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "core.h"
#include "platform.h"

static clockid_t global_clock;
static u64 global_freq;

void init_time() {
	global_clock = CLOCK_REALTIME;
	global_freq = 1000000000;

#if defined(_POSIX_MONOTONIC_CLOCK)
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		global_clock = CLOCK_MONOTONIC;
	}
#endif
}

u64 get_frequency() {
	return global_freq;
}

u64 get_time() {
	struct timespec ts;

	clock_gettime(global_clock, &ts);
	return (u64)ts.tv_sec * global_freq + (u64)ts.tv_nsec;
}

struct dir_iter {
	char root[1024];

	DIR* dir;

	struct dir_entry entry;
};

struct dir_iter* new_dir_iter(const char* dir_name) {
	DIR* dir = opendir(dir_name);

	if (!dir) {
		fprintf(stderr, "Failed to open directory: `%s'", dir_name);
		return null;
	}

	struct dir_iter* it = core_calloc(1, sizeof(struct dir_iter));

	strcpy(it->root, dir_name);
	it->dir = dir;

	dir_iter_next(it);

	return it;
}

void free_dir_iter(struct dir_iter* it) {
	closedir(it->dir);
	core_free(it);
}

struct dir_entry* dir_iter_cur(struct dir_iter* it) {
	return &it->entry;
}

bool dir_iter_next(struct dir_iter* it) {
	struct dirent* e = readdir(it->dir);

	if (!e) { return false; }

	if (strcmp(e->d_name, ".")  == 0) { return dir_iter_next(it); }
	if (strcmp(e->d_name, "..") == 0) { return dir_iter_next(it); }

	strcpy(it->entry.name, it->root);

	if (it->entry.name[strlen(it->entry.name) - 1] != '/') {
		strcat(it->entry.name, "/");
	}

	strcat(it->entry.name, e->d_name);

	it->entry.iter = it;

	return true;
}

bool file_exists(const char* name) {
	struct stat s;
	return (stat(name, &s) != -1);
}

bool file_is_regular(const char* name) {
	struct stat s;
	if (stat(name, &s) != -1) {
		return S_ISREG(s.st_mode);
	}

	return false;
}

bool file_is_dir(const char* name) {
	struct stat s;
	if (stat(name, &s) != -1) {
		return S_ISDIR(s.st_mode);
	}

	return false;
}

u64 file_mod_time(const char* name) {
	struct stat s;
	if (stat(name, &s) != -1) {
		return s.st_mtime;
	}

	return 0;
}
