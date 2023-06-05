/* repl.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>
#include "mpc.h"

#define ASSERT(val, cond, err) if (!(cond)) { val_del(val); return val_err(err); }

enum {
  VAL_NUM,
  VAL_ERR,
  VAL_SYM,
  VAL_SEXPR,
  VAL_QEXPR
};

enum {
  ERR_DIV_ZERO,
  ERR_BAD_FUNCTION,
  ERR_BAD_NUM,
  ERR_BAD_SEXPR,
  ERR_NON_NUM_ARG,
  ERR_WRONG_NUM_ARGS,
  ERR_WRONG_ARG_TYPE,
  ERR_EMPTY_ARGS,
  ERR_UNKNOWN
};

typedef struct Val {
  int type;
  long num;
  char* sym;
  char* err;
  int count;
  struct Val** cell;
} Val;

Val* val_read(mpc_ast_t* t);
Val* val_eval(Val* v);
void val_print(Val* v);
void val_println(Val* v);

/*
 * Val builders
 */

Val* val_num(long n) {
  Val* v = malloc(sizeof(Val));
  v->type = VAL_NUM;
  v->num = n;
  return v;
}

Val* val_err(int e) {
  Val* v = malloc(sizeof(Val));
  v->type = VAL_ERR;
  char* msg;
  switch (e) {
    case ERR_DIV_ZERO:
      msg = "division by zero";
      break;
    case ERR_BAD_FUNCTION:
      msg = "unknown function";
      break;
    case ERR_BAD_NUM:
      msg = "unable to parse number";
      break;
    case ERR_BAD_SEXPR:
      msg = "s-expression does not start with symbol";
      break;
    case ERR_NON_NUM_ARG:
      msg = "cannot operate on non-number";
      break;
    case ERR_WRONG_NUM_ARGS:
      msg = "wrong number of args provided";
      break;
    case ERR_WRONG_ARG_TYPE:
      msg = "wrong type in args";
      break;
    case ERR_EMPTY_ARGS:
      msg = "args required, received zero";
      break;
    default:
      msg = "unknown error";
  }
  v->err = malloc(strlen(msg) + 1);
  strcpy(v->err, msg);
  return v;
}

Val* val_sym(char* s) {
  Val* v = malloc(sizeof(Val));
  v->type = VAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

Val* val_sexpr(void) {
  Val* v = malloc(sizeof(Val));
  v->type = VAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

Val* val_qexpr(void) {
  Val* v = malloc(sizeof(Val));
  v->type = VAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

/*
 * Val operators
 */

void val_del(Val* v) {
  switch (v->type) {
    case VAL_NUM: break;
    case VAL_ERR: free(v->err); break;
    case VAL_SYM: free(v->sym); break;
    case VAL_SEXPR:
    case VAL_QEXPR:
      for (int i=0; i < v->count; i++) {
        val_del(v->cell[i]);
      }
      free(v->cell);
      break;
  }

  free(v);
}

Val* val_append(Val* v, Val* c) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(Val*) * v->count);
  v->cell[v->count - 1] = c;
  return v;
}

Val* val_pop(Val* v, int i) {
  Val* c = v->cell[i];
  memmove(&v->cell[i], &v->cell[i+1], sizeof(Val*) * v->count - i - 1);
  v->count--;
  v->cell = realloc(v->cell, sizeof(Val*) * v->count);
  return c;
}

Val* val_take(Val* v, int i) {
  Val* c = val_pop(v, i);
  val_del(v);
  return c;
}

Val* val_join(Val* a, Val* b) {
  while (b->count) {
    a = val_append(a, val_pop(b, 0));
  }
  val_del(b);
  return a;
}

/*
 * Val evaluators
 */

Val* builtin_head(Val* args) {
  ASSERT(args, args->count == 1, ERR_WRONG_NUM_ARGS);
  ASSERT(args, args->cell[0]->type == VAL_QEXPR, ERR_WRONG_ARG_TYPE);
  ASSERT(args, args->cell[0]->count != 0, ERR_EMPTY_ARGS);
  Val* v = val_take(args, 0);
  while (v->count > 1) {
    val_del(val_pop(v, 1));
  }
  return v;
}

Val* builtin_tail(Val* args) {
  ASSERT(args, args->count == 1, ERR_WRONG_NUM_ARGS);
  ASSERT(args, args->cell[0]->type == VAL_QEXPR, ERR_WRONG_ARG_TYPE);
  ASSERT(args, args->cell[0]->count != 0, ERR_EMPTY_ARGS);
  Val* v = val_take(args, 0);
  val_del(val_pop(v, 0));
  return v;
}

Val* builtin_list(Val* args) {
  args->type = VAL_QEXPR;
  return args;
}

Val* builtin_eval(Val* args) {
  ASSERT(args, args->count == 1, ERR_WRONG_NUM_ARGS);
  ASSERT(args, args->cell[0]->type == VAL_QEXPR, ERR_WRONG_ARG_TYPE);
  Val* v = val_take(args, 0);
  v->type = VAL_SEXPR;
  return val_eval(v);
}

Val* builtin_join(Val* args) {
  for (int i = 0; i < args->count; i++) {
    ASSERT(args, args->cell[i]->type == VAL_QEXPR, ERR_WRONG_ARG_TYPE);
  }
  Val* v = val_pop(args, 0);
  while (args->count) {
    v = val_join(v, val_pop(args, 0));
  }
  val_del(args);
  return v;
}

Val* builtin_op(Val* v, char* op) {
  for (int i=0; i < v->count; i++) {
    if (v->cell[i]->type != VAL_NUM) {
      val_del(v);
      return val_err(ERR_NON_NUM_ARG);
    }
  }

  Val* a = val_pop(v, 0);

  if ((strcmp(op, "-") == 0) && v->count == 0) {
    a->num *= -1;
  }

  while (v->count > 0) {
    Val* b = val_pop(v, 0);

    if (strstr("/%", op) && b->num == 0) {
      val_del(a);
      val_del(b);
      a = val_err(ERR_DIV_ZERO);
      break;
    }

    if (strcmp(op, "+") == 0) { a->num += b->num; }
    if (strcmp(op, "-") == 0) { a->num -= b->num; }
    if (strcmp(op, "*") == 0) { a->num *= b->num; }
    if (strcmp(op, "/") == 0) { a->num /= b->num; }
    if (strcmp(op, "%") == 0) { a->num %= b->num; }
    if (strcmp(op, "min") == 0 && b->num < a->num) {
      a->num = b->num;
    }
    if (strcmp(op, "max") == 0 && b->num > a->num) {
      a->num = b->num;
    }

    val_del(b);
  }

  val_del(v);
  return a;
}

Val* builtin(Val* v, char* func) {
  if (strcmp(func, "head") == 0) { return builtin_head(v); }
  if (strcmp(func, "tail") == 0) { return builtin_tail(v); }
  if (strcmp(func, "list") == 0) { return builtin_list(v); }
  if (strcmp(func, "eval") == 0) { return builtin_eval(v); }
  if (strcmp(func, "join") == 0) { return builtin_join(v); }
  if (strstr("+-*/%minmax", func)) { return builtin_op(v, func); }
  val_del(v);
  return val_err(ERR_BAD_FUNCTION);
}

Val* val_eval_sexpr(Val* v) {
  if (v->count == 0) { return v; }

  for (int i=0; i < v->count; i++) {
    v->cell[i] = val_eval(v->cell[i]);
  }

  if (v->count == 1) { return val_take(v, 0); }

  for (int i=0; i < v->count; i++) {
    if (v->cell[i]->type == VAL_ERR) { return val_take(v, i); }
  }

  Val* vs = val_pop(v, 0);
  if (vs->type != VAL_SYM) {
    val_del(vs);
    val_del(v);
    return val_err(ERR_BAD_SEXPR);
  }

  Val* r = builtin(v, vs->sym);
  val_del(vs);

  return r;
}

Val* val_eval(Val* v) {
  if (v->type == VAL_SEXPR) { return val_eval_sexpr(v); }
  return v;
}

/*
 * Val readers
 */

Val* val_read_num(mpc_ast_t* t) {
  errno = 0;
  long n = strtol(t->contents, NULL, 10);
  return errno == ERANGE ? val_err(ERR_BAD_NUM) : val_num(n);
}

int skip_append(mpc_ast_t* t) {
  return (
    strcmp(t->contents, "(") == 0 ||
    strcmp(t->contents, ")") == 0 ||
    strcmp(t->contents, "{") == 0 ||
    strcmp(t->contents, "}") == 0 ||
    strcmp(t->tag, "regex") == 0
  );
}

Val* val_read(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) { return val_read_num(t); }
  if (strstr(t->tag, "symbol")) { return val_sym(t->contents); }

  Val* root = NULL;
  if (strcmp(t->tag, ">") == 0) { root = val_sexpr(); }
  if (strstr(t->tag, "sexpr")) { root = val_sexpr(); }
  if (strstr(t->tag, "qexpr")) { root = val_qexpr(); }

  for (int i=0; i < t->children_num; i++) {
    if (skip_append(t->children[i])) { continue; }
    root = val_append(root, val_read(t->children[i]));
  }
  return root;
}

/*
 * Val printers
 */

void val_expr_print(Val* v, char open, char close) {
  putchar(open);
  for (int i=0; i < v->count; i++) {
    val_print(v->cell[i]);
    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void val_print(Val* v) {
  switch (v->type) {
    case VAL_NUM:
      printf("%li", v->num);
      break;
    case VAL_SYM:
      printf("%s", v->sym);
      break;
    case VAL_SEXPR:
      val_expr_print(v, '(', ')');
      break;
    case VAL_QEXPR:
      val_expr_print(v, '{', '}');
      break;
    case VAL_ERR:
      printf("**error** %s", v->err);
      break;
  }
}

void val_println(Val* v) {
  val_print(v);
  putchar('\n');
}

/*
 * Control functions
 */

void startup_info(void) {
  puts("its-lisp v0.1\nctrl-c to exit\n");
}

int should_exit(char* input) {
  int exit = strncmp(input, "exit", 4);
  int quit = strncmp(input, "quit", 4);
  return exit == 0 || quit == 0;
}

void process_input(char* input, mpc_parser_t* lisp) {
  mpc_result_t r;
  if (mpc_parse("<stdin>", input, lisp, &r)) {
    // mpc_ast_print(r.output);
    Val* v = val_eval(val_read(r.output));
    val_println(v);
    val_del(v);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

int main(int argc, char** argv) {
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* SExpr = mpc_new("sexpr");
  mpc_parser_t* QExpr = mpc_new("qexpr");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* ItsLisp = mpc_new("its_lisp");

  mpca_lang(
    MPCA_LANG_DEFAULT,
    "                                                         \
      number    : /-?[0-9]+/ ;                                \
      symbol    : '+' | '-' | '*' | '/' | '%'                 \
                | \"min\" | \"max\" | \"list\" | \"head\"     \
                | \"tail\" | \"join\" | \"eval\" | \"cons\"   \
                | \"len\" ;                                   \
      sexpr     : '(' <expr>* ')' ;                           \
      qexpr     : '{' <expr>* '}' ;                           \
      expr      : <number> | <symbol> | <sexpr> | <qexpr> ;   \
      its_lisp  : /^/ <expr>* /$/ ;                           \
    ",
    Number,
    Symbol,
    SExpr,
    QExpr,
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

  mpc_cleanup(6,
    Number,
    Symbol,
    SExpr,
    QExpr,
    Expr,
    ItsLisp
  );
  return 0;
}

