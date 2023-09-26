#include "ast.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "dump.h"
#include "vm.h"

typedef struct Parser Parser;
void parser(Ast *ast, const char *filename);


int main() {
  Ast *ast = AstNew();
  parser(ast, "test.rinha");

  assert(ast->root->kind == N_FILE);
VM vm;
  VMInit(&vm);

  Function toplevel;

  Compiler* compiler = CompilerNew(&vm);
  CompilerEmitFile(compiler, ast->root, &toplevel);

  DumpHex(toplevel.chunk.code, toplevel.chunk.size);

  VMExec(&vm, &toplevel);


  ArenaDestroy(ast->arena);
}
