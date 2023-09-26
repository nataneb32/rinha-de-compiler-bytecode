#ifndef COMPILER_H
#define COMPILER_H

#include "arena.h"
#include "ast.h"
#include "vm.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	const char* name;
	int8_t offset;
	bool isCaptured;
	bool isParam;
	bool isUpvalue;
	uint8_t depth;
} Local;

typedef struct Scope {
	// TODO: dynamic resize
	Local locals[2056];
	uint64_t localsSize;
	BytecodeChunk* chunk;
	struct Scope* parent;
	uint64_t stackSize;
	uint8_t upvalues[255];
	uint8_t upvaluesCount;
} Scope;

typedef struct {
	VM *vm;
	Arena *arena;
} Compiler;

Compiler *CompilerNew(VM *vm);

void CompilerEmitFile(Compiler *compiler, AstNode* node,  Function *toplevel);
int8_t CompilerEmitTerm(Compiler* compiler, Scope *scope, AstNode* node);

#endif
