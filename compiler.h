#ifndef COMPILER_H
#define COMPILER_H

#include "arena.h"
#include "ast.h"
#include "bytecode.h"
#include <stdint.h>

typedef struct {
	const char* name;
	uint8_t offset;
} Local;

typedef struct Scope {
	// TODO: dynamic resize
	Local locals[2056];
	uint64_t localsSize;
	BytecodeChunk* chunk;
	struct Scope* parent;
	uint64_t stackSize;
} Scope;

typedef struct {
	VM *vm;
	Arena *arena;
} Compiler;

Compiler *CompilerNew(VM *vm);

void CompilerEmitFile(Compiler *compiler, AstNode* node);
void CompilerEmitTerm(Compiler* compiler, Scope *scope, AstNode* node);

#endif
