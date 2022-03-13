#include <time.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <pthread.h>
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

const char* get_root_dir() {
	return "/";
}

struct dir_iter {
	char root[1024];

	DIR* dir;

	struct dir_entry entry;
};

struct dir_iter* new_dir_iter(const char* dir_name) {
	DIR* dir = opendir(dir_name);

	if (!dir) {
		fprintf(stderr, "Failed to open directory: `%s'\n", dir_name);
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

struct thread {
	pthread_t handle;
	void* uptr;
	bool working;
	thread_worker worker;
};

void* _thread_worker(void* ptr) {
	((struct thread*)ptr)->working = true;

	((struct thread*)ptr)->worker(ptr);

	((struct thread*)ptr)->working = false;

	return null;
}

struct thread* new_thread(thread_worker worker) {
	struct thread* thread = core_calloc(1, sizeof(struct thread));

	thread->worker = worker;

	return thread;
}

void free_thread(struct thread* thread) {
	thread_join(thread);
	core_free(thread);
}

void thread_execute(struct thread* thread) {
	if (thread->working) {
		fprintf(stderr, "Warning: Thread already active!\n");
		return;
	}

	pthread_create(&thread->handle, null, _thread_worker, thread);
}

void thread_join(struct thread* thread) {
	if (!thread->handle) { return; }

	pthread_join(thread->handle, null);
	thread->handle = 0;
}

bool thread_active(struct thread* thread) {
	return thread->working;
}

void* get_thread_uptr(struct thread* thread) {
	return thread->uptr;
}

void set_thread_uptr(struct thread* thread, void* ptr) {
	thread->uptr = ptr;
}

struct mutex {
	pthread_mutex_t m;

	void* data;
};

struct mutex* new_mutex(u64 size) {
	struct mutex* mutex = core_calloc(1, sizeof(struct mutex));

	pthread_mutex_init(&mutex->m, null);

	if (size > 0) {
		mutex->data = core_calloc(1, size);
	}

	return mutex;
}

void free_mutex(struct mutex* mutex) {
	pthread_mutex_destroy(&mutex->m);

	if (mutex->data) {
		core_free(mutex->data);
	}

	core_free(mutex);
}

void lock_mutex(struct mutex* mutex) {
	pthread_mutex_lock(&mutex->m);
}

void unlock_mutex(struct mutex* mutex) {
	pthread_mutex_unlock(&mutex->m);
}

void* mutex_get_ptr(struct mutex* mutex) {
	return mutex->data;
}
