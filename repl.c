/* repl.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>
#include "mpc.h"

enum { RESULT_NUM, RESULT_ERR };
enum { ERR_DIV_ZERO, ERR_BAD_OP, ERR_BAD_NUM, ERR_UNKNOWN };

typedef struct {
  int type;
  long value;
  int error;
} Result;

Result result_num(long v) {
  Result r = {.type=RESULT_NUM, .value=v};
  return r;
}

Result result_err(int e) {
  Result r = {.type=RESULT_ERR, .error=e};
  return r;
}

char* result_err_msg(Result r) {
  switch (r.error) {
    case ERR_DIV_ZERO:
      return "error: division by zero";
    case ERR_BAD_OP:
      return "error: unknown operator";
    case ERR_BAD_NUM:
      return "error: unable to parse number";
  }
  return "unknown error";
}

void result_print(Result r) {
  switch (r.type) {
    case RESULT_NUM:
      printf("~>  %li", r.value);
      break;
    case RESULT_ERR:
      printf("%s", result_err_msg(r));
      break;
  }
  putchar('\n');
}

void startup_info(void) {
  puts("its-lisp v0.1\nctrl-c to exit\n");
}

int should_exit(char* input) {
  int exit = strncmp(input, "exit", 4);
  int quit = strncmp(input, "quit", 4);
  return exit == 0 || quit == 0;
}

Result eval_op(char* op, Result a, Result b) {
  if (a.type == RESULT_ERR) { return a; }
  if (b.type == RESULT_ERR) { return b; }
  if (strcmp(op, "+") == 0) { return result_num(a.value + b.value); }
  if (strcmp(op, "-") == 0) { return result_num(a.value - b.value); }
  if (strcmp(op, "*") == 0) { return result_num(a.value * b.value); }
  if (strcmp(op, "/") == 0) {
    return b.value == 0 ?
      result_err(ERR_DIV_ZERO) :
      result_num(a.value / b.value);
  }
  return result_err(ERR_BAD_OP);
}

Result eval(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    errno = 0;
    long value = strtol(t->contents, NULL, 10);
    return errno == ERANGE ?
      result_err(ERR_BAD_NUM) :
      result_num(value);
  }

  char* op = t->children[1]->contents;
  Result result = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    result = eval_op(op, result, eval(t->children[i]));
    i++;
  }

  return result;
}

void process_input(char* input, mpc_parser_t* lisp) {
  mpc_result_t r;
  if (mpc_parse("<stdin>", input, lisp, &r)) {
    result_print(eval(r.output));
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

int main(int argc, char** argv) {
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* ItsLisp = mpc_new("its_lisp");

  mpca_lang(
    MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      its_lisp : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number,
    Operator,
    Expr,
    ItsLisp
  );

  startup_info();

  while (1) {
    char* input = readline("its> ");
    if (should_exit(input)) break;
    add_history(input);
    process_input(input, ItsLisp);
  }

  mpc_cleanup(4, Number, Operator, Expr, ItsLisp);
  return 0;
}

