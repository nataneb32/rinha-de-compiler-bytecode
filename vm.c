#include "vm.h"
#include "arena.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void BytecodeChunkInit(BytecodeChunk *chunk) {
	chunk->capacity = 0xf;
	chunk->code = (Bytecode*)malloc(sizeof(Bytecode) * chunk->capacity);
	chunk->size = 0;
}

void BytecodeChunkPush(BytecodeChunk *chunk, Bytecode byte) {
	if(chunk->size >= chunk->capacity) {
		chunk->capacity *= 2;
		chunk->code = (Bytecode*)realloc(chunk->code,chunk->capacity);
		assert(chunk->code);
	}

	chunk->code[chunk->size++] = byte;
}

void BytecodeChunkPushValue(BytecodeChunk *chunk, Value value) {
	Bytecode *code = (Bytecode*)&value;
	BytecodeChunkPush(chunk, code[0]);
	BytecodeChunkPush(chunk, code[1]);
	BytecodeChunkPush(chunk, code[2]);
	BytecodeChunkPush(chunk, code[3]);
	BytecodeChunkPush(chunk, code[4]);
	BytecodeChunkPush(chunk, code[5]);
	BytecodeChunkPush(chunk, code[6]);
	BytecodeChunkPush(chunk, code[7]);
	BytecodeChunkPush(chunk, code[8]);
	BytecodeChunkPush(chunk, code[9]);
	BytecodeChunkPush(chunk, code[10]);
	BytecodeChunkPush(chunk, code[11]);
	BytecodeChunkPush(chunk, code[12]);
	BytecodeChunkPush(chunk, code[13]);
	BytecodeChunkPush(chunk, code[14]);
	BytecodeChunkPush(chunk, code[15]);
}

void VMInit(VM* vm) {
	memset(vm,0,sizeof(VM));
	vm->arena = ArenaNew(0);
}

static bool CallframeHasNext(CallFrame* frame) {
	if(frame->function) {
		return frame->ip < frame->function->chunk.size;
	}
	if(frame->closure) {
		return frame->ip < frame->closure->function->chunk.size;
	}

	return false;
}

static uint8_t CallframeReadByte(CallFrame* frame) {
	if(frame->function) {
		return frame->function->chunk.code[frame->ip++];
	}
	if(frame->closure) {
		return frame->closure->function->chunk.code[frame->ip++];
	}

	return 0;
}

static Value CallframeReadValue(CallFrame* frame) {
	Value v;
	Bytecode *b = (Bytecode*)&v;

	b[0] = CallframeReadByte(frame);
	b[1] = CallframeReadByte(frame);
	b[2] = CallframeReadByte(frame);
	b[3] = CallframeReadByte(frame);
	b[4] = CallframeReadByte(frame);
	b[5] = CallframeReadByte(frame);
	b[6] = CallframeReadByte(frame);
	b[7] = CallframeReadByte(frame);
	b[8] = CallframeReadByte(frame);
	b[9] = CallframeReadByte(frame);
	b[10] = CallframeReadByte(frame);
	b[11] = CallframeReadByte(frame);
	b[12] = CallframeReadByte(frame);
	b[13] = CallframeReadByte(frame);
	b[14] = CallframeReadByte(frame);
	b[15] = CallframeReadByte(frame);

	return v;
}

Value* UnwrapUpvalue(Value* v) {
	if(v->type == V_UPVALUE) {
		Upvalue *u = (Upvalue*)v->obj;
		if(u->openValue) {
			return u->openValue;
		}

		return &u->closed;
	}

	return v;
}

void VMExec(VM *vm, Function *toplevel) {
	CallFrame *frame = &vm->callStack[vm->callStackSize++];
	frame->function = toplevel;
	frame->locals = vm->stack;
	vm->stackSize += frame->function->numOfLocals;

	outer: 
	while(vm->callStackSize > 0) {
		CallFrame *frame = &vm->callStack[vm->callStackSize - 1];
		while(CallframeHasNext(frame)) {
			Bytecode op = CallframeReadByte(frame);
			
			switch(op) {
				case B_ASSIGN_VALUE: {
					int8_t local = CallframeReadByte(frame);
					Value* v = &frame->locals[local];
					*v = CallframeReadValue(frame);
				}
				continue;
				case B_PRINT: {
					int8_t local = CallframeReadByte(frame);
					Value* v = UnwrapUpvalue(&frame->locals[local]);
					switch(v->type) {
						case V_INT:
							printf("%d\n", v->_int);
					}
				}
				continue;
				case B_COPY: {
					int8_t local1 = CallframeReadByte(frame);
					int8_t local2 = CallframeReadByte(frame);
					Value* v1 = UnwrapUpvalue(&frame->locals[local1]);
					Value* v2 = UnwrapUpvalue(&frame->locals[local2]);

					*v1 = *v2;
				}
				continue;
				case B_MUL: {
					int8_t local1 = CallframeReadByte(frame);
					int8_t local2 = CallframeReadByte(frame);
					Value* v1 = UnwrapUpvalue(&frame->locals[local1]);
					Value* v2 = UnwrapUpvalue(&frame->locals[local2]);

					assert(v1->type == V_INT && v2->type == V_INT);

					v1->_int *= v2->_int;
				}
				continue;
				case B_LT: {
					int8_t local1 = CallframeReadByte(frame);
					int8_t local2 = CallframeReadByte(frame);
					Value* v1 = UnwrapUpvalue(&frame->locals[local1]);
					Value* v2 = UnwrapUpvalue(&frame->locals[local2]);

					assert(v1->type == V_INT && v2->type == V_INT);

					v1->_int = v1->_int < v2->_int;
				}
				continue;
				case B_SUB: {
					int8_t local1 = CallframeReadByte(frame);
					int8_t local2 = CallframeReadByte(frame);
					Value* v1 = UnwrapUpvalue(&frame->locals[local1]);
					Value* v2 = UnwrapUpvalue(&frame->locals[local2]);

					assert(v1->type == V_INT && v2->type == V_INT);

					v1->_int -= v2->_int;
				}
				continue;
				case B_ADD: {
					int8_t local1 = CallframeReadByte(frame);
					int8_t local2 = CallframeReadByte(frame);
					Value* v1 = UnwrapUpvalue(&frame->locals[local1]);
					Value* v2 = UnwrapUpvalue(&frame->locals[local2]);

					assert(v1->type == V_INT && v2->type == V_INT);

					v1->_int += v2->_int;
				}
				continue;
				case B_CLOSURE: {
					int8_t function = CallframeReadByte(frame);
					Value* v = UnwrapUpvalue(&frame->locals[function]);
					assert(v->type == V_FUNCTION);
					Function *f = v->function;

					Closure *c = VMAllocObj(vm, sizeof(Closure));
					c->function = f;
					c->upvalues = malloc(sizeof(Upvalue*) * f->numOfUpvalues);

					for(int i = 0; i < f->numOfUpvalues;i++) {
						int8_t local = CallframeReadByte(frame);
						Value* capture = UnwrapUpvalue(&frame->locals[local]);
						Upvalue *u = VMAllocObj(vm, sizeof(Upvalue));
						u->openValue = capture;
						c->upvalues[i] = u;
					}
					
					frame->locals[function] = (Value){
						.type = V_CLOSURE,
						.obj = &c->obj,
					};
				}
				continue;
				case B_IFJMP: {
					int8_t local = CallframeReadByte(frame);
					uint8_t size = CallframeReadByte(frame);
					Value* cnd = UnwrapUpvalue(&frame->locals[local]);
					if(cnd->_int) {
						frame->ip += size;
					}
				} continue;
				case B_JMP: {
					uint8_t size = CallframeReadByte(frame);
					frame->ip += size;
				} continue;
				case B_CALL: {
					int8_t local = CallframeReadByte(frame);
					Value* callee = UnwrapUpvalue(&frame->locals[local]);
					CallFrame *newFrame = &vm->callStack[vm->callStackSize++];

					Function* f = NULL;
					Closure *c = NULL;

					if(callee->type == V_CLOSURE) {
						c = (Closure*)callee->obj;
						f = c->function;
					} else if(callee->type == V_FUNCTION) {
						f = callee->function;
					} else {
						printf("%d\n", callee->type);
						assert(false && "trying to call an invalid value");
					}

					newFrame->function = f;
					newFrame->ip = 0;

					vm->stackSize += f->numOfUpvalues;
					newFrame->locals = &vm->stack[vm->stackSize];

					vm->stackSize += f->numOfParams;
					vm->stackSize += f->numOfLocals;

					for(int i = 0; i < f->numOfParams; i++) {
						int8_t param = CallframeReadByte(frame);
						newFrame->locals[i] = frame->locals[param];
					}

					for(int i = 0; i < f->numOfUpvalues; i++) {
						newFrame->locals[- i - 1] = (Value){
							.type = V_UPVALUE,
							.obj = &c->upvalues[i]->obj,
						};
					}

					int8_t result = CallframeReadByte(frame);
					newFrame->result = &frame->locals[result];
				}
				goto outer;
			}
		}
		vm->stackSize -= frame->function->numOfUpvalues;
		vm->stackSize -= frame->function->numOfParams;
		vm->stackSize -= frame->function->numOfLocals;

		if(frame->result) {
			*frame->result = frame->locals[frame->function->numOfParams + frame->function->numOfLocals - 1];
		}

		vm->callStackSize--;
	}
}

Obj* VMAllocObj(VM *vm, size_t size) {
	Obj* o = malloc(size);
	return o;
}

void FunctionInit(Function* f) {
	BytecodeChunkInit(&f->chunk);
	f->numOfLocals = 0;
	f->numOfParams = 0;
	f->numOfUpvalues = 0;
}
