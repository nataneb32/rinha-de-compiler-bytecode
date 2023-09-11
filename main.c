#include "ast.h"
#include <assert.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include "bytecode.h"

typedef struct Parser {
} Parser;
void parser(Ast *ast, const char *filename);

typedef struct LuaEmitter {
  char *code;
  int size;
} LuaEmitter;

LuaEmitter *LuaEmitterNew() {
  LuaEmitter *emitter = malloc(sizeof(LuaEmitter));
  emitter->code = calloc(sizeof(char), 5000);
  emitter->size = 0;
  return emitter;
}

void LuaEmitterPushCodef(LuaEmitter *emitter, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int n = vsprintf(&emitter->code[emitter->size], format, ap);
  emitter->size += n;
}
void LuaEmitterEmit(LuaEmitter *emitter, AstNode *node) {
  switch (node->kind) {
  case N_FILE:
    return LuaEmitterEmit(emitter, node->file.expression);
  case N_LET:
    LuaEmitterPushCodef(emitter, "(function(%s) return ", node->let.name);
    LuaEmitterEmit(emitter, node->let.next);
    LuaEmitterPushCodef(emitter, " end)(");
    LuaEmitterEmit(emitter, node->let.value);
    LuaEmitterPushCodef(emitter, ")");
    return;
  case N_FUNCTION:
    LuaEmitterPushCodef(emitter, "function(");
    LuaEmitterEmit(emitter, node->function.parameters);
    LuaEmitterPushCodef(emitter, ")");
    LuaEmitterEmit(emitter, node->function.value);
    LuaEmitterPushCodef(emitter, "end");
    return;
  case N_ARGUMENT:
    if (node->argument.next) {
      LuaEmitterEmit(emitter, node->argument.value);
      LuaEmitterPushCodef(emitter, ",");
      LuaEmitterEmit(emitter, node->argument.next);
    } else {
      LuaEmitterEmit(emitter, node->argument.value);
    }
    return;
  case N_PARAMETER:
    if (node->parameter.next) {
      LuaEmitterPushCodef(emitter, "%s,", node->parameter.text);
    } else {
      LuaEmitterPushCodef(emitter, "%s", node->parameter.text);
    }
    return;
  case N_IF:
    LuaEmitterPushCodef(emitter, "(function() if ");
    LuaEmitterEmit(emitter, node->_if.condition);
    LuaEmitterPushCodef(emitter, " then return ");
    LuaEmitterEmit(emitter, node->_if.then);
    LuaEmitterPushCodef(emitter, " else return ");
    LuaEmitterEmit(emitter, node->_if.otherwise);
    LuaEmitterPushCodef(emitter, " end)()");
    return;
  case N_BOOL:
    if (node->bool.value) {
      LuaEmitterPushCodef(emitter, "true");
    } else {
      LuaEmitterPushCodef(emitter, "false");
    }
	return;
  case N_PRINT:
	LuaEmitterPushCodef(emitter, "print(");
	LuaEmitterEmit(emitter, node->print.value);
	LuaEmitterPushCodef(emitter, ")");
	return;
  }
}

int main() {
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  Ast *ast = AstNew();
  parser(ast, "test.rinha");

  assert(ast->root->kind == N_FILE);

  LuaEmitter *emitter = LuaEmitterNew();
  LuaEmitterEmit(emitter, ast->root);

  printf("%s\n", emitter->code);
  printf("%d\n", OP_MOV);
  ArenaDestroy(ast->arena);
}
