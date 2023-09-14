#ifndef BYTECODE_H
#define BYTECODE_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef uint16_t BytecodeOp;
enum BytecodeOp {
	BC_ADD = 0x01,
	BC_SUB,
	BC_MUL,
	BC_DIV,
	BC_PRINT,
	BC_PUSH,
	BC_POP,
	BC_GET_LOCAL,
	BC_SET_LOCAL,
	BC_CLOSURE = 0x10,
	BC_CMP =0x20,
	BC_CALL = 0xF0,
	BC_RET,
	BC_JMP_IF_TRUE = 0x0F,
};

typedef struct {
	uint8_t *bytes;
	int size;
	int capacity;
} BytecodeChunk;

// VM Objects

typedef struct VMObjHeader {
	enum {
		OBJ_STRING,
		OBJ_FUNCTION
	} type;
	struct VMObjHeader *next;
} VMObjHeader;

typedef struct {
	VMObjHeader header;
	BytecodeChunk* chunk;
	int arity; // Number of arguments
} VMFunction;

typedef struct {
	VMFunction *function;
} VMClosure;

typedef struct {
	VMObjHeader header;
	int size;
	char data[];
} VMString;

typedef struct {
	int stackOffset;
	BytecodeChunk *chunk;
	uint32_t ip;
} CallFrame;

enum {
	V_INT,
	V_OBJ,
};

typedef struct  __attribute__((__packed__)) {
	uint8_t type;
	union {
		int64_t _int;
		VMObjHeader* obj;
	};
} VMValue ;

static_assert(sizeof(VMValue) == sizeof(uint64_t) + sizeof(uint8_t), "invalid value size");

#define AS_INT(value) (VMValue){.type = V_INT, ._int = value}
#define AS_OBJ(value) (VMValue){.type = V_OBJ, .obj = value}

#define STACK_MAX 0xFFFF
typedef struct {
	VMValue stack[STACK_MAX];
	CallFrame callStack[STACK_MAX];
	uint64_t callStackSize;
	BytecodeChunk *toplevel;

	VMObjHeader *objects;
	VMObjHeader *lastObject;
	VMValue *constants;
	int constantsSize;

	uint32_t ip;
	uint32_t sp;
} VM;

BytecodeChunk *NewBytecodeChunk();
void BytecodeChunkAppendByte(BytecodeChunk *chunk, uint8_t byte);

VM *NewVM();
void VMEval(VM* vm);
VMObjHeader *VMAllocateObj(VM *vm, int type, size_t size);

#endif
