#include "bytecode.h"
#include "dump.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

BytecodeChunk *NewBytecodeChunk() {
	BytecodeChunk *chunk = malloc(sizeof(BytecodeChunk));
	chunk->size = 0;
	chunk->capacity = 0xFF;
	chunk->bytes = malloc(sizeof(uint64_t) * chunk->capacity);
	return chunk;
}

void BytecodeChunkAppendByte(BytecodeChunk *chunk, uint8_t byte) {
	if(chunk->size + 1 >= chunk->capacity) {
		chunk->capacity *= 2;
		void * tmp = realloc(chunk->bytes, chunk->capacity);
		if(tmp == NULL) {
			perror("failed to reallocate chunk");
		}

		chunk->bytes = tmp;
	}

	chunk->bytes[chunk->size++] = byte;
}

VM *NewVM() {
	VM* vm = calloc(1, sizeof(VM));
	vm->ip = 0;
	vm->sp = 0;
	vm->objects = NULL;
	vm->lastObject = NULL;
	return vm;
}

VMValue VMPopValue(VM *vm) {
	assert(vm->sp && "trying to pop a value from a empty stack");
	return vm->stack[--vm->sp];
}

void VMPushValue(VM *vm, VMValue value) {
	vm->stack[vm->sp++] = value;
}

VMValue VMConsumeValue(VM *vm, CallFrame* frame) {
	VMValue v;
	uint8_t *bytes = (uint8_t*)&v;
	bytes[0] = frame->chunk->bytes[frame->ip++];
	bytes[1] = frame->chunk->bytes[frame->ip++];
	bytes[2] = frame->chunk->bytes[frame->ip++];
	bytes[3] = frame->chunk->bytes[frame->ip++];
	bytes[4] = frame->chunk->bytes[frame->ip++];
	bytes[5] = frame->chunk->bytes[frame->ip++];
	bytes[6] = frame->chunk->bytes[frame->ip++];
	bytes[7] = frame->chunk->bytes[frame->ip++];
	bytes[8] = frame->chunk->bytes[frame->ip++];
	printf("type: %d\n", v.type);
	return v;
}

uint8_t VMReadByte(VM* vm, CallFrame* frame) {
	return frame->chunk->bytes[frame->ip++];
}

void VMGetLocal(VM* vm, CallFrame* frame, uint8_t offset) {
	VMValue v = vm->stack[frame->stackOffset + offset];
	printf("int: %d\n", v._int);
	VMPushValue(vm, v);
}

VMValue VMPeek(VM *vm, int8_t n) {
	return vm->stack[vm->sp - 1 - n];
}

void VMEval(VM *vm) {
	BytecodeOp op;
	CallFrame *frame;

	while(vm->callStackSize>0) {
		frame = &vm->callStack[vm->callStackSize-1];
		if(frame->chunk->size <= frame->ip) {
			vm->callStackSize--;
			continue;
		}
		op = frame->chunk->bytes[frame->ip++];

		printf("op: %d\n", op);
		switch(op) {
			case BC_PUSH:
			{
				VMPushValue(vm, VMConsumeValue(vm, frame));
			}
			continue;
			case BC_ADD:
			{
				VMValue v1 = VMPopValue(vm);
				VMValue v2 = VMPopValue(vm);
				assert(v2.type == V_INT);
				assert(v1.type == V_INT);
				VMPushValue(vm, AS_INT(v1._int + v2._int));
			}
			continue;
			case BC_PRINT: 
			{
				VMValue v = VMPeek(vm, 0);
				switch (v.type) {
					case V_INT:
						printf("%d\n", v._int);
						return;
					case V_OBJ:
						switch(v.obj->type) {
							case OBJ_STRING: {
								VMString *s = v.obj;
								printf("%s\n", s->data);
							}
							return;
							default:
								assert(0 && "invalid obj");
						}
					default:
						assert(0 && "unimplemented");
				}
			} 
			continue;
			case BC_GET_LOCAL:
				VMGetLocal(vm, frame,VMReadByte(vm, frame));
				continue;
			case BC_CALL:{
				VMValue v = VMPeek(vm, 0);
				assert(v.type == V_OBJ);
				VMFunction *f = v.obj;

				CallFrame *newFrame = &vm->callStack[vm->callStackSize++];
				newFrame->chunk = f->chunk;
				newFrame->stackOffset = vm->sp;
				newFrame->ip = 0;

				for(int i = 0; i < f->arity; i++) {
					VMValue v = VMPeek(vm, i + 1);
					VMPushValue(vm, v);
				}

				DumpHex(vm->stack, vm->sp * sizeof(VMValue));
			}
			continue;
		}
	}
}

VMObjHeader *VMAllocateObj(VM *vm, int type, size_t size) {
	VMObjHeader *header = calloc(1, size);
	header->type = type;

	if(!vm->objects) {
		vm->objects = header;
	}

	if(vm->lastObject) {
		vm->lastObject->next = header;
	}

	vm->lastObject = header;
	return header;
}
