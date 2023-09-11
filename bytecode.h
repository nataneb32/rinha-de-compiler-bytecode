#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>

typedef uint16_t BytecodeOp;
#define OP_MOV (BytecodeOp){0x1}
#define OP_STORE (BytecodeOp){0x1}

#endif
