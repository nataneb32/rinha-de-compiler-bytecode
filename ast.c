#include "ast.h"
#include <stdlib.h>

Ast* AstNew() {
	Arena* arena = ArenaNew(1024);
	Ast* ast = ArenaAlloc(arena, sizeof(Arena));
	ast->arena = arena;
	ast->root = NULL;
	return ast;
}

AstNode* AstBuildNode(Ast* ast, enum AstNodeKind kind) {
	AstNode* node = ArenaAlloc(ast->arena, sizeof(AstNode));
	node->next = NULL;
	node->kind = kind;
	return node;
}
