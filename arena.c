#include "arena.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Arena* ArenaNew(int capacity) {
	Arena* arena = malloc(sizeof(Arena));
	arena->cap = capacity;
	arena->data = malloc(capacity);
	arena->next = NULL;
	arena->size = 0;
	return arena;
}

void* ArenaAlloc(Arena* arena, int size) {
	if(arena->size + size >= arena->cap) {
		if(!arena->next) {
			arena->next = ArenaNew(arena->cap);
		}
		return ArenaAlloc(arena->next, size);
	}

	void* ptr = &arena->data[arena->size];
	arena->size += size;
	memset(ptr, 0, size);
	return ptr;
}

void ArenaDestroy(Arena* arena) {
	if(arena->next) {
		ArenaDestroy(arena->next);
	}
	
	free(arena->data);
	free(arena);
}


char* ArenaCpyStr(Arena* arena, const  char* src) {
	uint64_t len = strlen(src);
	char* s = ArenaAlloc(arena, len+1);
	assert(s);
	strncpy(s, src, len);
	return s;
}

