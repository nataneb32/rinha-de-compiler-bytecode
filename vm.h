#ifndef VM_H
#define VM_H

#include "arena.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t Bytecode;

enum {
	B_NOP = 0,
	B_ADD = 1,
	B_PRINT = 2,
	B_SUB = 3,
	B_MUL = 4,
	B_LT = 5,
	B_ASSIGN_VALUE = 0x20,
	B_COPY = 0x21,
	B_CLOSURE = 0x30,
	B_IFJMP = 0xa0,
	B_JMP = 0xa1,
	B_CALL = 0xf0,
};

typedef struct {
	Bytecode *code;
	uint32_t size;
	uint32_t capacity;
} BytecodeChunk;

typedef struct Obj {
	struct Obj* next;
} Obj;

typedef struct {
	BytecodeChunk chunk;
	uint32_t numOfLocals;
	uint32_t numOfParams;
	uint32_t numOfUpvalues;
} Function;

typedef struct {
	enum {
		V_INT,
		V_STR,
		V_UPVALUE,
		V_FUNCTION,
		V_CLOSURE,
	} type;
	union {
		int _int;
		Obj *obj;
		Function* function;
	};
} Value;

static_assert(sizeof(Value) == sizeof(Bytecode)*16, "wrong value size");

typedef struct {
	Obj obj;
	Value* openValue;
	Value closed;
} Upvalue;

/*
Stack of a callframe

+----------+
| Upvalues | } number of upvalues
+----------+
| Params   | } number of params
+----------+
| Locals   | } number of locals
+----------+

This means that the offset of the first local is the number of upvalues plus
the number of params
*/

typedef struct {
	Obj obj;
	Function *function;
	Upvalue **upvalues;
} Closure;

typedef struct {
	Value* locals;
	uint32_t ip;
	Function *function;
	Closure *closure;
	Value *result;
} CallFrame;

#define STACK_MAX 0xffff

typedef struct {
	Value stack[STACK_MAX];
	uint16_t stackSize;
	CallFrame callStack[STACK_MAX];
	uint16_t callStackSize;
	Obj* objects;
	Obj* lastObj;
	Arena *arena;
} VM;

void BytecodeChunkInit(BytecodeChunk *chunk);
void BytecodeChunkPush(BytecodeChunk *chunk, Bytecode byte);
void BytecodeChunkPushValue(BytecodeChunk *chunk, Value value);

void VMInit(VM *vm);
void VMExec(VM *vm, Function *toplevel);
Obj* VMAllocObj(VM *vm, size_t size);

void FunctionInit(Function* f);

#endif
