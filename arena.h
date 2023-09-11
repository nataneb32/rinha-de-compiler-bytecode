#ifndef ARENA_H
#define ARENA_H

typedef struct Arena {
	char* data;
	int cap;
	int size;
	struct Arena* next;
} Arena;

Arena* ArenaNew(int capacity);
void *ArenaAlloc(Arena* arena, int size);
void ArenaDestroy(Arena* arena);
// Utils

char* ArenaCpyStr(Arena* arena, const  char* src);
#endif
