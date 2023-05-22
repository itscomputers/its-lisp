/* repl.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>
#include "mpc.h"

mpc_parser_t* Number;
mpc_parser_t* Operator;
mpc_parser_t* Expr;
mpc_parser_t* ItsLisp;

void startup_info(void) {
  puts("its-lisp v0.1\nctrl-c to exit\n");
}

int should_exit(char* input) {
  int exit = strncmp(input, "exit", 4);
  int quit = strncmp(input, "quit", 4);
  return exit == 0 || quit == 0;
}

long eval_op(char* op, int a, int b) {
  if (strcmp(op, "+") == 0) { return a + b; }
  if (strcmp(op, "-") == 0) { return a - b; }
  if (strcmp(op, "*") == 0) { return a * b; }
  if (strcmp(op, "/") == 0) { return a / b; }
  return 0;
}

long eval(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  char* op = t->children[1]->contents;
  long result = eval(t->children[2]);

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
    printf("~>  %li\n", eval(r.output));
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
      exp     : <number> | '(' <operator> <expr>+ ')' ;  \
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

