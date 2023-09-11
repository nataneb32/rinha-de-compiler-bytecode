#ifndef AST_H
#define AST_H

#include "arena.h"
typedef struct {
	int start;
	int end;
	const char* file;
} AstNodeLoc;

typedef struct AstNode {
	enum AstNodeKind {
		N_FILE,
		N_LET,
		N_SYMBOL,
		N_STR,
		N_BOOL,
		N_INT,
		N_BINARY,
		N_CALL,
		N_FUNCTION,
		N_BUILTIN_FUN,
		N_IF,
		N_PARAMETER,
		N_ARGUMENT,
		N_FIRST,
		N_SECOND,
		N_PRINT,
		N_TUPLE
	} kind;

	struct AstNode *next;

	union {
		struct {
			const char *name;
			struct AstNode *position;
			struct AstNode *expression;
			AstNodeLoc location;
		} file;

		struct {
			const char *name;
			AstNodeLoc nameLocation;
			struct AstNode *value;
			struct AstNode *kind;
			struct AstNode *next;
			AstNodeLoc location;
		} let;

		struct {
			struct AstNode *condition;
			struct AstNode *then;
			struct AstNode *otherwise;
			AstNodeLoc location;
		} _if;
		struct {
			const char *value;
			AstNodeLoc location;
		} str;
		struct {
			int value;
			AstNodeLoc location;
		} bool;

		struct {
			int value;
			AstNodeLoc location;
		} _int;

		struct {
			struct AstNode* lhs;
			struct AstNode* rhs;
			enum AstBinaryOp {
				OP_ADD,
			} op;
			AstNodeLoc location;
		} binary;

		struct {
			const char *text;
			AstNodeLoc location;
			struct AstNode *next;
		} parameter;

		struct {
			struct AstNode *value;
			struct AstNode *next;
		} argument;

		struct {
			struct AstNode *callee;
			struct AstNode *arguments;
			AstNodeLoc location;
		} call;
		struct {
			struct AstNode *value;
			struct AstNode *parameters;
			AstNodeLoc location;
		} function;

		struct {
			struct AstNode* value;
			AstNodeLoc location;
		} print;

		struct {
			struct AstNode* value;
			AstNodeLoc location;
		} first;

		struct {
			struct AstNode* value;
			AstNodeLoc location;
		} second;

		struct {
			struct AstNode* first;
			struct AstNode* second;
			AstNodeLoc location;
		} tuple;

		struct {
			const char *text;
			AstNodeLoc location;
		} var;

	};
} AstNode;

typedef struct {
	Arena *arena;
	AstNode* root;
} Ast;

Ast* AstNew();
AstNode* AstBuildNode(Ast* ast, enum AstNodeKind kind);

#endif
