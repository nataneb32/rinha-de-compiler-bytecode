#include "compiler.h"
#include "arena.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include "bytecode.h"
#include "dump.h"

Compiler *CompilerNew(VM *vm) {
	Arena *arena = ArenaNew(sizeof(Compiler));
	Compiler *c = ArenaAlloc(arena, sizeof(Compiler));
	c->vm = vm;
	c->arena = arena;
	
	return c;
}
#if 0
void emit_binary(VM *vm, BytecodeChunk* chunk, AstNode* node);
void emit_push(VM *vm, BytecodeChunk* chunk, VMValue value);

void emit_value(VM *vm, BytecodeChunk* chunk, AstNode* node) {
	switch (node->kind) {
		case N_PRINT: {
				emit_value(vm, chunk, node->print.value);
				BytecodeChunkAppendByte(chunk, BC_PRINT);
			}
			return;
		case N_BINARY:
			emit_binary(vm, chunk, node);
			return;
		case N_INT:
			emit_push(vm, chunk, AS_INT(node->_int.value));
			return;
		case N_STR: {
			VMString *s = (VMString*)VMAllocateObj(vm, OBJ_STRING, sizeof(VMString)+strlen(node->str.value));
			strcpy(s->data, node->str.value);
			VMValue v =  AS_OBJ(&s->header);
			DumpHex(&v, sizeof(VMValue));
			emit_push(vm, chunk, v);
		}
			return;
		case N_LET: {
		}
		return;
		case N_FUNCTION: 
		default:
			assert(0 && "Not implemuint32_tented");
	}
}


#endif

void ScopeInit(Scope* scope, BytecodeChunk* chunk) {
	scope->chunk = chunk;
	scope->localsSize = 0;
}

void CompilerEmitPush(Compiler* compiler, Scope *scope, VMValue value) {
	BytecodeChunk* chunk = scope->chunk;
	uint8_t* ptr = (uint8_t*)&value;
	BytecodeChunkAppendByte(chunk, BC_PUSH);
	BytecodeChunkAppendByte(chunk, ptr[0]);
	BytecodeChunkAppendByte(chunk, ptr[1]);
	BytecodeChunkAppendByte(chunk, ptr[2]);
	BytecodeChunkAppendByte(chunk, ptr[3]);
	BytecodeChunkAppendByte(chunk, ptr[4]);
	BytecodeChunkAppendByte(chunk, ptr[5]);
	BytecodeChunkAppendByte(chunk, ptr[6]);
	BytecodeChunkAppendByte(chunk, ptr[7]);
	BytecodeChunkAppendByte(chunk, ptr[8]);
}

void CompilerEmitFile(Compiler *compiler, AstNode* node) {
	assert(node->kind == N_FILE);
	
	Scope fileScope;
	ScopeInit(&fileScope, NewBytecodeChunk());

	CallFrame* global = &compiler->vm->callStack[compiler->vm->callStackSize++];
	global->stackOffset = 0;
	global->chunk = fileScope.chunk;
	global->ip = 0;

	CompilerEmitTerm(compiler, &fileScope, node->file.expression);

	compiler->vm->toplevel = fileScope.chunk;
}

void CompilerEmitPrint(Compiler *compiler, Scope* scope,  AstNode* node) {
	assert(node->kind == N_PRINT);

	CompilerEmitTerm(compiler, scope, node->print.value);
	
	BytecodeChunkAppendByte(scope->chunk, BC_PRINT);
}

void CompilerPushLocal(Compiler *compiler, Scope *scope, const char* name) {
	Local *local = &scope->locals[scope->localsSize];
	local->name = name;
	local->offset = scope->localsSize++;
}

void CompilerEmitLet(Compiler *compiler, Scope* scope,  AstNode* node) {
	assert(node->kind == N_LET);
	printf("let: %s\n", node->let.name);
	// Push value to stack
	// Add local variable
	// Compile expression
	CompilerEmitTerm(compiler, scope, node->let.value);
	CompilerPushLocal(compiler, scope, ArenaCpyStr(compiler->arena, node->let.name));
	CompilerEmitTerm(compiler, scope, node->let.next);
}

void CompilerEmitVar(Compiler *compiler, Scope* scope,  AstNode* node) {
	assert(node->kind == N_VAR);
	printf("%s\n", node->var.text);
	Local *local = NULL;

	for(int i = scope->localsSize-1; i >= 0; i--) {
		if(strcmp(scope->locals[i].name, node->var.text) == 0) {
			local = &scope->locals[i];
			break;
		}
	}

	assert(local && "Undefined variable");

	BytecodeChunkAppendByte(scope->chunk, BC_GET_LOCAL);
	BytecodeChunkAppendByte(scope->chunk, local->offset);
}


void CompilerEmitBinary(Compiler *compiler, Scope* scope,  AstNode* node) {
	assert(node->kind == N_BINARY);
	switch(node->binary.op) {
		case OP_ADD:
			CompilerEmitTerm(compiler, scope, node->binary.lhs);
			CompilerEmitTerm(compiler, scope, node->binary.rhs);
			BytecodeChunkAppendByte(scope->chunk, BC_ADD);
			return;
	}
}

void CompilerEmitFunction(Compiler *compiler, Scope* scope,  AstNode* node) {
	assert(node->kind == N_FUNCTION);
	VMFunction *f = (VMFunction*)VMAllocateObj(compiler->vm, OBJ_FUNCTION, sizeof(VMFunction));
	f->chunk = NewBytecodeChunk();
	f->arity = 0;

	Scope functionScope;
	ScopeInit(&functionScope, f->chunk);
	
	AstNode *param = node->function.parameters;
	while(param) {
		f->arity++;
		printf("%d\n", f->arity);
		CompilerPushLocal(compiler, &functionScope, ArenaCpyStr(compiler->arena, param->parameter.text));
		param = param->parameter.next;
	}
	
	CompilerEmitTerm(compiler, &functionScope, node->function.value);
	CompilerEmitPush(compiler, scope, AS_OBJ(&f->header));
}


void CompilerEmitCall(Compiler *compiler, Scope* scope,  AstNode* node) {
	assert(node->kind == N_CALL);
	
	AstNode *arg = node->call.arguments;

	while(arg) {
		printf("args\n");
		CompilerEmitTerm(compiler, scope, arg->argument.value);
		arg = arg->argument.next;
	}
	
	CompilerEmitTerm(compiler, scope, node->call.callee);

	BytecodeChunkAppendByte(scope->chunk, BC_CALL);
}


void CompilerEmitTerm(Compiler *compiler, Scope* scope,  AstNode* node) {
	
	switch(node->kind) {
		case N_INT:
			CompilerEmitPush(compiler, scope, AS_INT(node->_int.value));
			return;
		case N_PRINT:
			CompilerEmitPrint(compiler, scope, node);
			return;
		case N_LET:
			CompilerEmitLet(compiler, scope, node);
			return;
		case N_VAR:
			CompilerEmitVar(compiler, scope, node);
		return;
		case N_BINARY:
			CompilerEmitBinary(compiler, scope, node);
		return;
		case N_FUNCTION:
			CompilerEmitFunction(compiler, scope, node);
			return;
		case N_CALL:
			CompilerEmitCall(compiler, scope, node);
			return;
		default:
			printf("unknown %d\n", node->kind);
			assert(0 && "Unexpected");
	}
}
