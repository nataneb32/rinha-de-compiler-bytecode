#include "arena.h"
#include "ast.h"
#include "json.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Parser {
} Parser;

static char *readAllToString(FILE *file) {
  char result[100] = {0};
  int size = 1;
  char *out = NULL;
  while (fgets(result, 100, file) != NULL) {
    int bufsize = strlen(result);
    char *new = realloc(out, size + bufsize);
    assert(new);
    out = new;
    strcpy(&out[size - 1], result);
    size += bufsize;
    out[size] = '\0';
  }

  return out;
}

static char *rinha2json(const char *filename) {
  char cmd[1024];
  int charsCount = snprintf(cmd, 1024, "./rinha2json %s", filename);
  assert(charsCount < 1024);
  FILE *json = popen(cmd, "r");

  char *a = readAllToString(json);
  if (a == NULL) {
    return NULL;
  }

  return a;
}

void parse_file(Ast *ast, json_value *file);
AstNode *parse_term(Ast *ast, json_value *term);
AstNode *parse_let(Ast *ast, json_value *let);
AstNode *parse_if(Ast *ast, json_value *_if);
AstNode *parse_str(Ast *ast, json_value *term);
AstNodeLoc parse_location(Ast *ast, json_value *location);
AstNode *parse_bool(Ast *ast, json_value *bool);
AstNode *parse_int(Ast *ast, json_value *_int);
AstNode *parse_binary(Ast *ast, json_value *binary);
AstNode *parse_call(Ast *ast, json_value *call);
enum AstBinaryOp parse_binaryop(Ast *ast, json_value *op);
AstNode *parse_parameters(Ast *ast, json_value *parameters);
AstNode *parse_arguments(Ast *ast, json_value *arguments);
AstNode *parse_function(Ast *ast, json_value *function);
AstNode *parse_print(Ast *ast, json_value *print);
AstNode *parse_first(Ast *ast, json_value *first);
AstNode *parse_second(Ast *ast, json_value *second);
AstNode *parse_tuple(Ast *ast, json_value *tuple);
AstNode *parse_var(Ast *ast, json_value *var);

void parser(Ast *ast, const char *filename) {
  char *json = rinha2json(filename);
  printf("json: %d %s\n", strlen(json), json);
  json_value *jsonValue = json_parse(json, strlen(json));
  assert(jsonValue);
  assert(jsonValue->type == json_object);
  parse_file(ast, jsonValue);
  free(json);
  json_value_free(jsonValue);
}

void parse_file(Ast *ast, json_value *file) {
  json_value name;
  json_value expression;
  json_value location;

  json_entry_by_name(file, &name, "name");
  json_entry_by_name(file, &location, "location");
  json_entry_by_name(file, &expression, "expression");

  AstNode *node = AstBuildNode(ast, N_FILE);
  ast->root = node;

  if (name.type == json_string) {
    node->file.name = ArenaCpyStr(ast->arena, name.u.string.ptr);
  } else {
    perror("invalid error");
  }
  assert(expression.type == json_object);
  node->file.expression = parse_term(ast, &expression);
}

AstNode *parse_term(Ast *ast, json_value *term) {
  assert(term->type == json_object);
  json_value kind;

  json_entry_by_name(term, &kind, "kind");
  assert(kind.type == json_string);
  if (strcmp("Let", kind.u.string.ptr) == 0) {
    return parse_let(ast, term);
  }
  if (strcmp("If", kind.u.string.ptr) == 0) {
    return parse_if(ast, term);
  }
  if (strcmp("Str", kind.u.string.ptr) == 0) {
    return parse_str(ast, term);
  }
  if (strcmp("Bool", kind.u.string.ptr) == 0) {
    return parse_bool(ast, term);
  }
  if (strcmp("Int", kind.u.string.ptr) == 0) {
    return parse_int(ast, term);
  }
  if (strcmp("Binary", kind.u.string.ptr) == 0) {
    return parse_binary(ast, term);
  }
  if (strcmp("Call", kind.u.string.ptr) == 0) {
    return parse_call(ast, term);
  }
  if (strcmp("Function", kind.u.string.ptr) == 0) {
    return parse_function(ast, term);
  }
  if (strcmp("Print", kind.u.string.ptr) == 0) {
    return parse_print(ast, term);
  }
  if (strcmp("First", kind.u.string.ptr) == 0) {
    return parse_first(ast, term);
  }
  if (strcmp("Second", kind.u.string.ptr) == 0) {
    return parse_second(ast, term);
  }
  if (strcmp("Tuple", kind.u.string.ptr) == 0) {
    return parse_tuple(ast, term);
  }
  if (strcmp("Var", kind.u.string.ptr) == 0) {
    return parse_var(ast, term);
  }
  printf("%d\n", strcmp("Let", kind.u.string.ptr));
  assert(0);
}

AstNode *parse_let(Ast *ast, json_value *term) {
  json_value name;
  json_value value;
  json_value next;
  json_value location;

  json_entry_by_name(term, &name, "name");
  json_entry_by_name(term, &value, "value");
  json_entry_by_name(term, &next, "next");

  AstNode *node = AstBuildNode(ast, N_LET);

  if (name.type == json_object) {
    json_value text;
    json_value textLocation;

    json_entry_by_name(&name, &text, "text");
    json_entry_by_name(&name, &textLocation, "location");

    node->let.name = ArenaCpyStr(ast->arena, text.u.string.ptr);
    node->let.nameLocation = parse_location(ast, &textLocation);
  } else {
    perror("invalid name");
  }

  if (value.type == json_object) {
    node->let.value = parse_term(ast, &value);
  } else {
    perror("invalid term");
  }

  if (next.type == json_object) {
    node->let.next = parse_term(ast, &next);
  } else {
    perror("invalid term");
  }

  return node;
}

AstNode *parse_if(Ast *ast, json_value *_if) {
  json_value condition;
  json_value then;
  json_value otherwise;
  json_value location;

  json_entry_by_name(_if, &condition, "condition");
  json_entry_by_name(_if, &then, "then");
  json_entry_by_name(_if, &otherwise, "otherwise");
  json_entry_by_name(_if, &location, "location");

  AstNode *node = AstBuildNode(ast, N_IF);

  if (condition.type == json_object) {
    node->_if.condition = parse_term(ast, &condition);
  } else {
    perror("invalid condition");
  }

  if (then.type == json_object) {
    node->_if.then = parse_term(ast, &then);
  } else {
    perror("invalid then");
  }

  if (otherwise.type == json_object) {
    node->_if.otherwise = parse_term(ast, &otherwise);
  } else {
    perror("invalid else");
  }

  if (location.type == json_object) {
    node->_if.location = parse_location(ast, &location);
  }
  return node;
}

AstNodeLoc parse_location(Ast *ast, json_value *location) {
  json_value filename;
  json_value start;
  json_value end;

  json_entry_by_name(location, &filename, "filename");
  json_entry_by_name(location, &start, "start");
  json_entry_by_name(location, &end, "end");

  assert(filename.type == json_string);
  assert(start.type == json_integer);
  assert(end.type == json_integer);

  return (AstNodeLoc){.file = ArenaCpyStr(ast->arena, filename.u.string.ptr),
                      .start = start.u.integer,
                      .end = end.u.integer};
}

AstNode *parse_str(Ast *ast, json_value *str) {
  json_value value;
  json_value location;

  json_entry_by_name(str, &value, "value");
  json_entry_by_name(str, &location, "location");

  AstNode *node = AstBuildNode(ast, N_STR);

  assert(value.type == json_string);

  node->str.value = ArenaCpyStr(ast->arena, value.u.string.ptr);
  node->str.location = parse_location(ast, &location);

  return node;
}

AstNode *parse_bool(Ast *ast, json_value *bool) {
  json_value value;
  json_value location;

  json_entry_by_name(bool, &value, "value");
  json_entry_by_name(bool, &location, "location");

  AstNode *node = AstBuildNode(ast, N_BOOL);

  if (value.type == json_boolean) {
    node->bool.value = value.u.boolean;
  } else {
    perror("invalid value");
  }

  if (location.type == json_object) {
    node->bool.location = parse_location(ast, &location);
  }
  return node;
}

AstNode *parse_int(Ast *ast, json_value *_int) {
  json_value value;
  json_value location;

  json_entry_by_name(_int, &value, "value");
  json_entry_by_name(_int, &location, "location");

  AstNode *node = AstBuildNode(ast, N_INT);

  if (value.type == json_integer) {
    node->_int.value = value.u.integer;
  } else {
    perror("invalid value");
  }

  if (location.type == json_object) {
    node->_int.location = parse_location(ast, &location);
  }

  return node;
}

AstNode *parse_binary(Ast *ast, json_value *binary) {
  json_value op;
  json_value lhs;
  json_value rhs;
  json_value location;

  json_entry_by_name(binary, &op, "op");
  json_entry_by_name(binary, &lhs, "lhs");
  json_entry_by_name(binary, &rhs, "rhs");
  json_entry_by_name(binary, &location, "location");

  AstNode *node = AstBuildNode(ast, N_BINARY);

  if (op.type == json_string) {
    node->binary.op = parse_binaryop(ast, &op);
  } else {
    perror("invalid op");
  }

  if (lhs.type == json_object) {
    node->binary.lhs = parse_term(ast, &lhs);
  } else {
    perror("invalid lhs");
  }

  if (rhs.type == json_object) {
    node->binary.rhs = parse_term(ast, &rhs);
  } else {
    perror("invalid else");
  }

  if (location.type == json_object) {
    node->binary.location = parse_location(ast, &location);
  }
  return node;
}

AstNode *parse_parameters(Ast *ast, json_value *parameters) {
  assert(parameters->type == json_array);
  AstNode *previous = NULL;
  AstNode *first = NULL;
  for (int i = 0; i < parameters->u.array.length; i++) {
    json_value *parameter = parameters->u.array.values[i];
    json_value text;
    json_value location;

    assert(parameter->type == json_object);
    json_entry_by_name(parameter, &text, "text");
    json_entry_by_name(parameter, &location, "location");

    assert(text.type == json_string);
    assert(location.type == json_object);

    AstNode *node = AstBuildNode(ast, N_PARAMETER);
	if(!first) {
		first = node;
	}
    if (previous) {
      previous->parameter.next = node;
    }

    node->parameter.text = ArenaCpyStr(ast->arena, text.u.string.ptr);
    node->parameter.location = parse_location(ast, &location);

    previous = node;
  }
  return first;
}

AstNode *parse_call(Ast *ast, json_value *call) {
  json_value callee;
  json_value arguments;
  json_value rhs;
  json_value location;

  json_entry_by_name(call, &callee, "callee");
  json_entry_by_name(call, &arguments, "arguments");
  json_entry_by_name(call, &rhs, "rhs");
  json_entry_by_name(call, &location, "location");

  AstNode *node = AstBuildNode(ast, N_CALL);

  if (callee.type == json_object) {
    node->call.callee = parse_term(ast, &callee);
  } else {
    perror("invalid callee");
  }

  if (arguments.type == json_array) {
    node->call.arguments = parse_arguments(ast, &arguments);
  } else {
    perror("invalid arguments");
  }

  if (location.type == json_object) {
    node->call.location = parse_location(ast, &location);
  }
  return node;
}

AstNode *parse_arguments(Ast *ast, json_value *arguments) {
  assert(arguments->type == json_array);
  AstNode *previous = NULL;
  AstNode *root = NULL;
  for (int i = 0; i < arguments->u.array.length; i++) {
    json_value *argument = arguments->u.array.values[i];

    AstNode *node = AstBuildNode(ast, N_ARGUMENT);
	if(!root) {
		root = node;
	}
    if (previous) {
      previous->argument.next = node;
    }

    node->argument.value = parse_term(ast, argument);
    previous = node;
  }
  return root;
}

AstNode *parse_function(Ast *ast, json_value *function) {
  assert(function->type == json_object);

  json_value parameters;
  json_value value;
  json_value location;

  json_entry_by_name(function, &parameters, "parameters");
  json_entry_by_name(function, &value, "value");
  json_entry_by_name(function, &location, "location");

  AstNode *node = AstBuildNode(ast, N_FUNCTION);

  node->function.parameters = parse_parameters(ast, &parameters);
  node->function.location = parse_location(ast, &location);
  node->function.value = parse_term(ast, &value);
  return node;
}

AstNode *parse_print(Ast *ast, json_value *print) {
  assert(print->type == json_object);

  json_value value;
  json_value location;

  json_entry_by_name(print, &value, "value");
  json_entry_by_name(print, &location, "location");

  AstNode *node = AstBuildNode(ast, N_PRINT);

  node->print.location = parse_location(ast, &location);
  node->print.value = parse_term(ast, &value);
  return node;
}

AstNode *parse_first(Ast *ast, json_value *first) {
  assert(first->type == json_object);

  json_value value;
  json_value location;

  json_entry_by_name(first, &value, "value");
  json_entry_by_name(first, &location, "location");
  AstNode *node = AstBuildNode(ast, N_FIRST);

  node->print.location = parse_location(ast, &location);
  node->print.value = parse_term(ast, &value);
  return node;
}

AstNode *parse_second(Ast *ast, json_value *second) {
  assert(second->type == json_object);

  json_value value;
  json_value location;

  json_entry_by_name(second, &value, "value");
  json_entry_by_name(second, &location, "location");

  AstNode *node = AstBuildNode(ast, N_SECOND);

  node->print.location = parse_location(ast, &location);
  node->print.value = parse_term(ast, &value);
  return node;
}

AstNode *parse_tuple(Ast *ast, json_value *tuple) {
  assert(tuple->type == json_object);

  json_value first;
  json_value second;
  json_value location;

  json_entry_by_name(tuple, &first, "first");
  json_entry_by_name(tuple, &location, "location");

  AstNode *node = AstBuildNode(ast, N_TUPLE);

  node->tuple.location = parse_location(ast, &location);
  node->tuple.first = parse_term(ast, &first);
  node->tuple.second = parse_term(ast, &second);
  return node;
}

AstNode *parse_var(Ast *ast, json_value *var) {
  assert(var->type == json_object);

  json_value text;
  json_value location;

  json_entry_by_name(var, &text, "text");
  json_entry_by_name(var, &location, "location");

  AstNode *node = AstBuildNode(ast, N_VAR);

  node->var.location = parse_location(ast, &location);
  node->var.text = ArenaCpyStr(ast->arena, text.u.string.ptr);
  return node;
}

enum AstBinaryOp parse_binaryop(Ast *ast, json_value *op) {
  assert(op->type == json_string);

  if (strcmp("Add", op->u.string.ptr) == 0) {
    return OP_ADD;
  }
  if (strcmp("Sub", op->u.string.ptr) == 0) {
    return OP_SUB;
  }
  if (strcmp("Mul", op->u.string.ptr) == 0) {
    return OP_MUL;
  }
  if (strcmp("Lt", op->u.string.ptr) == 0) {
    return OP_LT;
  }

  return 0;
}
