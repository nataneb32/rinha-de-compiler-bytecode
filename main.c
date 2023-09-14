#include "ast.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bytecode.h"
#include "compiler.h"
#include "dump.h"

typedef struct Parser Parser;
void parser(Ast *ast, const char *filename);


int main() {
  Ast *ast = AstNew();
  parser(ast, "test.rinha");

  assert(ast->root->kind == N_FILE);
  VM* vm = NewVM();

  Compiler* compiler = CompilerNew(vm);
  CompilerEmitFile(compiler, ast->root);

  DumpHex(vm->toplevel->bytes, vm->toplevel->size);

  VMEval(vm);

  ArenaDestroy(ast->arena);
}
