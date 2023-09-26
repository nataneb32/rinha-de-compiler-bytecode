#include "vm.h"
#include <stdio.h>
int main() {
	VM vm;
	Function toplevel;

	VMInit(&vm);
	VMExec(&vm, &toplevel.chunk);

	BytecodeChunkInit(&toplevel.chunk);

	Upvalue *u = VMAllocObj(&vm, sizeof(Upvalue));
	u->openValue = &vm.stack[0];

	BytecodeChunkPush(&toplevel.chunk, B_ASSIGN_VALUE);
	BytecodeChunkPush(&toplevel.chunk, 0);
	BytecodeChunkPushValue(&toplevel.chunk, (Value){
		.type = V_INT,
		._int = 5,
	});

	BytecodeChunkPush(&toplevel.chunk, B_ASSIGN_VALUE);
	BytecodeChunkPush(&toplevel.chunk, 1);
	BytecodeChunkPushValue(&toplevel.chunk, (Value){
		.type = V_UPVALUE,
		.obj = &u->obj,
	});

	BytecodeChunkPush(&toplevel.chunk, B_PRINT);
	BytecodeChunkPush(&toplevel.chunk, 0);
	BytecodeChunkPush(&toplevel.chunk, B_PRINT);
	BytecodeChunkPush(&toplevel.chunk, 1);

	toplevel.numOfLocals = 3;

	Function f;
	BytecodeChunkInit(&f.chunk);
	BytecodeChunkPush(&f.chunk, B_ASSIGN_VALUE);
	BytecodeChunkPush(&f.chunk, 1);
	BytecodeChunkPushValue(&f.chunk, (Value){
		.type = V_INT,
		._int = 1,
	});

	BytecodeChunkPush(&f.chunk, B_ADD);
	BytecodeChunkPush(&f.chunk, 0);
	BytecodeChunkPush(&f.chunk, 1);
	BytecodeChunkPush(&f.chunk, B_PRINT);
	BytecodeChunkPush(&f.chunk, 0);
	f.numOfUpvalues = 1;

	BytecodeChunkPush(&toplevel.chunk, B_ASSIGN_VALUE);
	BytecodeChunkPush(&toplevel.chunk, 2);
	BytecodeChunkPushValue(&toplevel.chunk, (Value){
		.type = V_FUNCTION,
		.function = &f,
	});

	BytecodeChunkPush(&toplevel.chunk, B_CLOSURE);
	BytecodeChunkPush(&toplevel.chunk, 2);
	BytecodeChunkPush(&toplevel.chunk, 2);
	BytecodeChunkPush(&toplevel.chunk, 1);

	BytecodeChunkPush(&toplevel.chunk, B_CALL);
	BytecodeChunkPush(&toplevel.chunk, 2);

	BytecodeChunkPush(&toplevel.chunk, B_CALL);
	BytecodeChunkPush(&toplevel.chunk, 2);

	BytecodeChunkPush(&toplevel.chunk, B_CALL);
	BytecodeChunkPush(&toplevel.chunk, 2);

	BytecodeChunkPush(&toplevel.chunk, B_PRINT);
	BytecodeChunkPush(&toplevel.chunk, 0);
	BytecodeChunkPush(&toplevel.chunk, B_PRINT);
	BytecodeChunkPush(&toplevel.chunk, 1);

	VMExec(&vm, &toplevel);
}
