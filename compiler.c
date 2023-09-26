#include "compiler.h"
#include "arena.h"
#include "ast.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "dump.h"
#include "vm.h"

Compiler *CompilerNew(VM *vm) {
	Arena *arena = ArenaNew(sizeof(Compiler));
	Compiler *c = ArenaAlloc(arena, sizeof(Compiler));
	c->vm = vm;
	c->arena = arena;
	
	return c;
}
uint8_t BinaryOpToBytecodeMap[] = {
	[OP_ADD] = B_ADD,
	[OP_SUB] = B_SUB,
	[OP_MUL] = B_MUL,
	[OP_LT] = B_LT,
};

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
	scope->parent = NULL;
	scope->upvaluesCount = 0;
}

uint8_t ScopeAddLocal(Compiler* compiler, Scope* scope, const char* name, int offset) {
	Local* local = &scope->locals[scope->localsSize];

	local->name = ArenaCpyStr(compiler->arena, name);
	local->offset = (scope->localsSize++) - scope->upvaluesCount;

	return local->offset;
}

Local *ScopeFindLocal(Compiler *compiler, Scope *scope, const char* name) {
	for(int i = scope->localsSize-1; i >= 0; i--) {
		if(scope->locals[i].name && strcmp(scope->locals[i].name, name) == 0) {
			return &scope->locals[i];
		}
	}

	return NULL;
}

Local *ScopeFindUpvalue(Compiler *compiler, Scope *scope, const char* name) {
	Local* local = ScopeFindLocal(compiler, scope, name);

	if(local == NULL) {
		if(scope->parent != NULL) {
			local = ScopeFindUpvalue(compiler, scope->parent, name);
			if(local != NULL) {
				local->isCaptured = true;
				
				scope->upvalues[scope->upvaluesCount] = local->offset;
				Local* up = &scope->locals[scope->localsSize];
				up->offset = -(++scope->upvaluesCount);
				up->name = local->name;

				return up;
			}
		}

		return NULL;
	}


	return local;
}

 void CompilerEmitFile(Compiler *compiler, AstNode* node,  Function *toplevel) {
	Scope scope;
	FunctionInit(toplevel);
	ScopeInit(&scope, &toplevel->chunk);

	CompilerEmitTerm(compiler, &scope, node->file.expression);
	toplevel->numOfLocals = scope.localsSize;
}

uint8_t CompilerEmitFunction(Compiler *compiler, Scope* scope,  AstNode* node) {
	Function* f =  ArenaAlloc(compiler->vm->arena, sizeof(Function));
	Scope fScope;
	FunctionInit(f);
	ScopeInit(&fScope, &f->chunk);
	fScope.parent = scope;


	BytecodeChunkPush(scope->chunk, B_ASSIGN_VALUE);
	BytecodeChunkPush(scope->chunk, scope->localsSize - scope->upvaluesCount);
	BytecodeChunkPushValue(scope->chunk, (Value){
		.type = V_FUNCTION,
		.function = f,
	});

	AstNode* param = node->function.parameters;
	
	while (param) {
		Local *l = &fScope.locals[fScope.localsSize];
		l->name = param->parameter.text;
		l->offset = fScope.localsSize++;
		param = param->parameter.next;
		f->numOfParams++;
	}

	CompilerEmitTerm(compiler, &fScope, node->function.value);
	f->numOfLocals = fScope.localsSize - f->numOfParams - fScope.upvaluesCount;

	if(fScope.upvaluesCount == 0) {
		return scope->localsSize++ - scope->upvaluesCount;
	}

	BytecodeChunkPush(scope->chunk, B_CLOSURE);
	BytecodeChunkPush(scope->chunk, scope->localsSize - scope->upvaluesCount);
	for(int i = 0; i < fScope.upvaluesCount; i++) {
		BytecodeChunkPush(scope->chunk, fScope.upvalues[i]);
	}
	
	f->numOfUpvalues = fScope.upvaluesCount;

	DumpHex(fScope.chunk->code, fScope.chunk->size);

	printf("upvalues: %d\n", fScope.upvaluesCount);

	return scope->localsSize++ - scope->upvaluesCount;
}

int8_t CompilerEmitTerm(Compiler *compiler, Scope* scope,  AstNode* node) {
	switch(node->kind) {
		case N_LET: {
			int8_t fixpoint = ScopeAddLocal(compiler, scope, ArenaCpyStr(compiler->arena, node->let.name), scope->localsSize - scope->upvaluesCount);
			int8_t value = CompilerEmitTerm(compiler, scope, node->let.value);
			BytecodeChunkPush(scope->chunk, B_COPY);
			BytecodeChunkPush(scope->chunk, fixpoint);
			BytecodeChunkPush(scope->chunk, value);
			return CompilerEmitTerm(compiler, scope, node->let.next);
		};
		case N_INT: {
			BytecodeChunkPush(scope->chunk, B_ASSIGN_VALUE);
			BytecodeChunkPush(scope->chunk, scope->localsSize - scope->upvaluesCount);
			BytecodeChunkPushValue(scope->chunk, (Value){
				.type = V_INT,
				._int = node->_int.value
			});
			return scope->localsSize++ - scope->upvaluesCount;
		}
		case N_PRINT: {
			uint8_t l = CompilerEmitTerm(compiler, scope, node->print.value);
			BytecodeChunkPush(scope->chunk, B_PRINT);
			BytecodeChunkPush(scope->chunk, l);
			return l;
		}
		case N_VAR: {
			Local *l = ScopeFindUpvalue(compiler, scope, node->var.text);
			assert(l && "Unable to find variable");
			return l->offset;
		}
		case N_FUNCTION: {
			return CompilerEmitFunction(compiler, scope, node);
		}
		case N_CALL: {
			uint8_t args[255];
			uint8_t argsCount = 0;
			AstNode* arg = node->call.arguments;
			while (arg != NULL)  {
				uint8_t l = CompilerEmitTerm(compiler, scope, arg->argument.value);
				args[argsCount++] = l;
				arg = arg->argument.next;
			}
			uint8_t f = CompilerEmitTerm(compiler, scope, node->call.callee);
			//CompilerEmitTerm(compiler, scope, node->function.parameters);
			BytecodeChunkPush(scope->chunk, B_CALL);
			BytecodeChunkPush(scope->chunk, f);
			for(int i =0 ; i < argsCount; i++) {
				BytecodeChunkPush(scope->chunk, args[i]);
			}
			BytecodeChunkPush(scope->chunk, scope->localsSize - scope->upvaluesCount);
			return scope->localsSize++ - scope->upvaluesCount;
		}

		case N_BINARY: {
			uint8_t lhs = CompilerEmitTerm(compiler, scope, node->binary.lhs);
			uint8_t rhs = CompilerEmitTerm(compiler, scope, node->binary.rhs);

			BytecodeChunkPush(scope->chunk, B_COPY);
			BytecodeChunkPush(scope->chunk, scope->localsSize - scope->upvaluesCount);
			BytecodeChunkPush(scope->chunk, lhs);

			BytecodeChunkPush(scope->chunk, BinaryOpToBytecodeMap[node->binary.op]);
			BytecodeChunkPush(scope->chunk, scope->localsSize - scope->upvaluesCount);
			BytecodeChunkPush(scope->chunk, rhs);
		} return scope->localsSize++ - scope->upvaluesCount;
		case N_IF: {
			int8_t cnd = CompilerEmitTerm(compiler, scope, node->_if.condition);
			BytecodeChunkPush(scope->chunk, B_IFJMP);
			BytecodeChunkPush(scope->chunk, cnd);
			int cndPos =  scope->chunk->size;
			BytecodeChunkPush(scope->chunk, 5);
			int8_t otherwise = CompilerEmitTerm(compiler, scope, node->_if.otherwise);
			BytecodeChunkPush(scope->chunk, B_JMP);
			int endThen = scope->chunk->size;
			scope->chunk->code[cndPos] = scope->chunk->size - cndPos;
			BytecodeChunkPush(scope->chunk, 4);
			int8_t then = CompilerEmitTerm(compiler, scope, node->_if.then);
			BytecodeChunkPush(scope->chunk, B_COPY);
			BytecodeChunkPush(scope->chunk, scope->localsSize - scope->upvaluesCount);
			BytecodeChunkPush(scope->chunk, then);
			BytecodeChunkPush(scope->chunk, B_JMP);
			BytecodeChunkPush(scope->chunk, 3);
			scope->chunk->code[endThen] = scope->chunk->size - endThen - 1;
			BytecodeChunkPush(scope->chunk, B_COPY);
			BytecodeChunkPush(scope->chunk, scope->localsSize - scope->upvaluesCount);
			BytecodeChunkPush(scope->chunk, otherwise);
		} return scope->localsSize++ - scope->upvaluesCount;
		default:
			printf("unknown %d\n", node->kind);
			assert(0 && "Unexpected");
	}
}
