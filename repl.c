/* repl.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>
#include "mpc.h"
#include "error.h"

#define ASSERT(val, cond, err) \
  if (!(cond)) { \
    val_del(val); \
    return val_err(err); \
  }
#define ASSERT_ARG_COUNT(val_to_del, val, expected) \
  if (val->count != expected) { \
    int given = val->count; \
    val_del(val_to_del); \
    Err *err = err_arg_count(expected, given); \
    return val_err(err); \
  }
#define ASSERT_ARG_TYPE(val_to_del, val, expected) \
  if (val->type != expected) { \
    char *given = type_name(val->type); \
    char *expected_name = type_name(expected); \
    val_del(val_to_del); \
    Err *err = err_arg_type(expected_name, given); \
    return val_err(err); \
  }
#define ASSERT_CELL_ARG_TYPE(val_to_del, val, index, expected) \
  if (val->cell[index]->type != expected) { \
    char *given = type_name(val->cell[index]->type); \
    char *expected_name = type_name(expected); \
    val_del(val_to_del); \
    Err *err = err_cell_arg_type(index, expected_name, given); \
    return val_err(err); \
  }

struct Val;
typedef struct Val Val;

struct Env;
typedef struct Env Env;

typedef Val*(*BuiltIn)(Env*, Val*);

struct Val {
  int type;

  long num;
  char *sym;
  Err *err;

  BuiltIn func;
  Env *env;
  Val *args;
  Val *body;

  int count;
  Val **cell;
};

struct Env {
  Env *parent;
  int count;
  char **syms;
  Val **vals;
};

enum {
  VAL_NUM,
  VAL_ERR,
  VAL_SYM,
  VAL_FUNC,
  VAL_SEXPR,
  VAL_QEXPR
};

char *type_name(int t) {
  switch (t) {
    case VAL_NUM: return "number";
    case VAL_ERR: return "error";
    case VAL_SYM: return "symbol";
    case VAL_FUNC: return "function";
    case VAL_SEXPR: return "s-expression";
    case VAL_QEXPR: return "q-expression";
  }
  return "unknown-type";
}

Val *val_read(mpc_ast_t *t);
Val *val_call(Env *e, Val *f, Val *v);
Val *val_eval(Env *e, Val *v);
void val_print(Val *v);
void val_println(Val *v);

Env *env_new(void);
Env *env_copy(Env *e);
void env_del(Env *e);
Val *env_get(Env *e, Val *k);
void env_def(Env *e, Val *k, Val *v);
void env_put(Env *e, Val *k, Val *v);

/*
 * Val builders
 */

Val *val_num(long n) {
  Val *v = malloc(sizeof(Val));
  v->type = VAL_NUM;
  v->num = n;
  return v;
}

Val *val_err(Err* err) {
  Val *v = malloc(sizeof(Val));
  v->type = VAL_ERR;
  v->err = malloc(sizeof(Err));
  v->err = err_copy(err);
  err_del(err);
  return v;
}

Val *val_sym(char *s) {
  Val *v = malloc(sizeof(Val));
  v->type = VAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

Val *val_func(BuiltIn func) {
  Val *v = malloc(sizeof(Val));
  v->type = VAL_FUNC;
  v->func = func;
  return v;
}

Val *val_lambda(Val *args, Val *body) {
  Val *v = malloc(sizeof(Val));
  v->type = VAL_FUNC;
  v->func = NULL;
  v->env = env_new();
  v->args = args;
  v->body = body;
  return v;
}

Val *val_sexpr(void) {
  Val *v = malloc(sizeof(Val));
  v->type = VAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

Val *val_qexpr(void) {
  Val *v = malloc(sizeof(Val));
  v->type = VAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

/*
 * Val operators
 */

void val_del(Val *v) {
  switch (v->type) {
    case VAL_NUM: break;
    case VAL_FUNC:
      if (!v->func) {
        env_del(v->env);
        val_del(v->args);
        val_del(v->body);
      }
      break;
    case VAL_ERR: free(v->err); break;
    case VAL_SYM: free(v->sym); break;
    case VAL_SEXPR:
    case VAL_QEXPR:
      for (int i = 0; i < v->count; i++) {
        val_del(v->cell[i]);
      }
      free(v->cell);
      break;
  }

  free(v);
}

Val *val_copy(Val *v) {
  Val *c = malloc(sizeof(Val));
  c->type = v->type;

  switch (v->type) {
    case VAL_NUM: c->num = v->num; break;
    case VAL_FUNC:
      if (v->func) {
        c->func = v->func;
      } else {
        c->func = NULL;
        c->env = env_copy(v->env);
        c->args = val_copy(v->args);
        c->body = val_copy(v->body);
      }
      break;
    case VAL_ERR:
      c->err = err_copy(v->err);
      break;
    case VAL_SYM:
      c->sym = malloc(strlen(v->sym) + 1);
      strcpy(c->sym, v->sym);
      break;
    case VAL_SEXPR:
    case VAL_QEXPR:
      c->count = v->count;
      c->cell = malloc(sizeof(Val*) * c->count);
      for (int i = 0; i < c->count; i++) {
        c->cell[i] = val_copy(v->cell[i]);
      }
    break;
  }

  return c;
}

Val *val_append(Val *v, Val *c) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(Val*) * v->count);
  v->cell[v->count - 1] = c;
  return v;
}

Val *val_pop(Val *v, int i) {
  Val *c = v->cell[i];
  memmove(&v->cell[i], &v->cell[i+1], sizeof(Val*) * v->count - i - 1);
  v->count--;
  v->cell = realloc(v->cell, sizeof(Val*) * v->count);
  return c;
}

Val *val_take(Val *v, int i) {
  Val *c = val_pop(v, i);
  val_del(v);
  return c;
}

Val *val_join(Val *a, Val *b) {
  while (b->count) {
    a = val_append(a, val_pop(b, 0));
  }
  val_del(b);
  return a;
}

/*
 * Val evaluators
 */

Val *builtin_head(Env *e, Val *args) {
  ASSERT_ARG_COUNT(args, args, 1);
  ASSERT_CELL_ARG_TYPE(args, args, 0, VAL_QEXPR);
  ASSERT(args, args->cell[0]->count > 0, err_empty_cell_args(0));
  Val *v = val_take(args, 0);
  while (v->count > 1) {
    val_del(val_pop(v, 1));
  }
  return v;
}

Val *builtin_tail(Env *e, Val *args) {
  ASSERT_ARG_COUNT(args, args, 1);
  ASSERT_CELL_ARG_TYPE(args, args, 0, VAL_QEXPR);
  ASSERT(args, args->cell[0]->count > 0, err_empty_cell_args(0));
  Val *v = val_take(args, 0);
  val_del(val_pop(v, 0));
  return v;
}

Val *builtin_list(Env *e, Val *args) {
  args->type = VAL_QEXPR;
  return args;
}

Val *builtin_eval(Env *e, Val *args) {
  ASSERT_ARG_COUNT(args, args, 1);
  ASSERT_CELL_ARG_TYPE(args, args, 0, VAL_QEXPR);
  Val *v = val_take(args, 0);
  v->type = VAL_SEXPR;
  return val_eval(e, v);
}

Val *builtin_join(Env *e, Val *args) {
  for (int i = 0; i < args->count; i++) {
    ASSERT_CELL_ARG_TYPE(args, args, i, VAL_QEXPR);
  }
  Val *v = val_pop(args, 0);
  while (args->count) {
    v = val_join(v, val_pop(args, 0));
  }
  val_del(args);
  return v;
}

Val *builtin_var(Env *e, Val *v, char *func) {
  ASSERT_CELL_ARG_TYPE(v, v, 0, VAL_QEXPR);
  Val *syms = v->cell[0];
  for (int i = 0; i < syms->count; i++) {
    ASSERT_CELL_ARG_TYPE(v, syms, i, VAL_SYM);
  }
  ASSERT_ARG_COUNT(v, v, syms->count + 1);

  for (int i = 0; i < syms->count; i++) {
    if (strcmp(func, "def")) {
      env_def(e, syms->cell[i], v->cell[i + 1]);
    } else {
      env_put(e, syms->cell[i], v->cell[i + 1]);
    }
  }

  val_del(v);
  return val_sexpr();
}

Val *builtin_def(Env *e, Val *v) {
  return builtin_var(e, v, "def");
}

Val *builtin_assign(Env *e, Val *v) {
  return builtin_var(e, v, ":=");
}

Val *builtin_lambda(Env *e, Val *v) {
  ASSERT_ARG_COUNT(v, v, 2);
  ASSERT_CELL_ARG_TYPE(v, v, 0, VAL_QEXPR);
  ASSERT_CELL_ARG_TYPE(v, v, 1, VAL_QEXPR);
  for (int i = 0; i < v->cell[0]->count; i++) {
    ASSERT_CELL_ARG_TYPE(v, v->cell[0], i, VAL_SYM);
  }

  Val *args = val_pop(v, 0);
  Val *body = val_pop(v, 0);
  val_del(v);

  return val_lambda(args, body);
}

Val *builtin_op(Env *e, Val *v, char *op);

Val *builtin_add(Env *e, Val *v) {
  return builtin_op(e, v, "+");
}

Val *builtin_sub(Env *e, Val *v) {
  return builtin_op(e, v, "-");
}

Val *builtin_mul(Env *e, Val *v) {
  return builtin_op(e, v, "*");
}

Val *builtin_div(Env *e, Val *v) {
  return builtin_op(e, v, "/");
}

Val *builtin_mod(Env *e, Val *v) {
  return builtin_op(e, v, "%");
}

Val *builtin_min(Env *e, Val *v) {
  return builtin_op(e, v, "min");
}

Val *builtin_max(Env *e, Val *v) {
  return builtin_op(e, v, "max");
}

Val *builtin_op(Env *e, Val *v, char *op) {
  for (int i=0; i < v->count; i++) {
    ASSERT_CELL_ARG_TYPE(v, v, i, VAL_NUM);
  }

  Val *a = val_pop(v, 0);

  if ((strcmp(op, "-") == 0) && v->count == 0) {
    a->num *= -1;
  }

  while (v->count > 0) {
    Val *b = val_pop(v, 0);

    if (strstr("/%", op) && b->num == 0) {
      val_del(a);
      val_del(b);
      a = val_err(err_new(ERR_ARITHMETIC, "division by zero"));
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

Val *val_eval_sexpr(Env *e, Val *v) {
  if (v->count == 0) { return v; }

  for (int i=0; i < v->count; i++) {
    v->cell[i] = val_eval(e, v->cell[i]);
  }

  if (v->count == 1) { return val_take(v, 0); }

  for (int i=0; i < v->count; i++) {
    if (v->cell[i]->type == VAL_ERR) { return val_take(v, i); }
  }

  Val *f = val_pop(v, 0);
  if (f->type != VAL_FUNC) {
    char *given = type_name(f->type);
    char *expected = type_name(VAL_FUNC);
    val_del(f);
    val_del(v);
    return val_err(err_cell_arg_type(0, expected, given));
  }

  Val *r = val_call(e, f, v);
  val_del(f);

  return r;
}

Val *val_eval(Env *e, Val *v) {
  if (v->type == VAL_SYM) {
    Val *x = env_get(e, v);
    val_del(v);
    return x;
  }
  if (v->type == VAL_SEXPR) { return val_eval_sexpr(e, v); }
  return v;
}

Val *val_call(Env *e, Val *f, Val *v) {
  if (f->func) { return f->func(e, v); }

  int given = v->count;
  int total = f->args->count;

  while (v->count) {
    ASSERT(
      v,
      f->args->count != 0,
      err_arg_count(total, given)
    );

    Val *sym = val_pop(f->args, 0);
    if (strcmp(sym->sym, "&") == 0) {
      ASSERT_ARG_COUNT(v, f->args, 1);
      Val *nsym = val_pop(f->args, 0);
      env_put(f->env, nsym, builtin_list(e, v));
      val_del(sym);
      val_del(nsym);
      break;
    }

    Val *val = val_pop(v, 0);
    env_put(f->env, sym, val);
    val_del(sym);
    val_del(val);
  }

  val_del(v);

  if (f->args->count > 0 && strcmp(f->args->cell[0]->sym, "&") == 0) {
    ASSERT_ARG_COUNT(v, f->args, 2);
    val_del(val_pop(f->args, 0));
    Val *sym = val_pop(f->args, 0);
    Val *val = val_qexpr();
    env_put(f->env, sym, val);
    val_del(sym);
    val_del(val);
  }

  if (f->args->count == 0) {
    f->env->parent = e;
    return builtin_eval(
      f->env,
      val_append(val_sexpr(), val_copy(f->body))
    );
  }
  return val_copy(f);
}

/*
 * Val readers
 */

Val *val_read_num(mpc_ast_t *t) {
  errno = 0;
  long n = strtol(t->contents, NULL, 10);
  return errno == ERANGE ?
    val_err(err_parse_number(t->contents)) :
    val_num(n);
}

int skip_append(mpc_ast_t *t) {
  return (
    strcmp(t->contents, "(") == 0 ||
    strcmp(t->contents, ")") == 0 ||
    strcmp(t->contents, "{") == 0 ||
    strcmp(t->contents, "}") == 0 ||
    strcmp(t->tag, "regex") == 0
  );
}

Val *val_read(mpc_ast_t *t) {
  if (strstr(t->tag, "number")) { return val_read_num(t); }
  if (strstr(t->tag, "symbol")) { return val_sym(t->contents); }

  Val *root = NULL;
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

void val_expr_print(Val *v, char open, char close) {
  putchar(open);
  for (int i=0; i < v->count; i++) {
    val_print(v->cell[i]);
    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void val_print(Val *v) {
  switch (v->type) {
    case VAL_NUM:
      printf("%li", v->num);
      break;
    case VAL_SYM:
      printf("%s", v->sym);
      break;
    case VAL_FUNC:
      if (v->func) {
        printf("<builtin>");
      } else {
        printf("(\\ ");
        val_print(v->args);
        putchar(' ');
        val_print(v->body);
        putchar(')');
      }
      break;
    case VAL_SEXPR:
      val_expr_print(v, '(', ')');
      break;
    case VAL_QEXPR:
      val_expr_print(v, '{', '}');
      break;
    case VAL_ERR:
      printf("**%s**: %s", v->err->name, v->err->det);
      break;
  }
}

void val_println(Val *v) {
  val_print(v);
  putchar('\n');
}

/*
 * Env operators
 */

Env *env_new(void) {
  Env *e = malloc(sizeof(Env));
  e->parent = NULL;
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

Env *env_copy(Env *e) {
  Env *c = malloc(sizeof(Env));
  c->parent = e->parent;
  c->count = e->count;
  c->syms = malloc(sizeof(char*) * c->count);
  c->vals = malloc(sizeof(Val*) * c->count);
  for (int i = 0; i < c->count; i++) {
    c->syms[i] = malloc(strlen(e->syms[i] + 1));
    strcpy(c->syms[i], e->syms[i]);
    c->vals[i] = val_copy(e->vals[i]);
  }
  return c;
}

void env_del(Env *e) {
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    val_del(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}

Val *env_get(Env *e, Val *k) {
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      return val_copy(e->vals[i]);
    }
  }
  if (e->parent) {
    return env_get(e->parent, k);
  }
  return val_err(err_unbound_symbol(k->sym));
}

void env_def(Env *e, Val *k, Val *v) {
  while (e->parent) { e = e->parent; }
  env_put(e, k, v);
}

void env_put(Env *e, Val *k, Val *v) {
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      val_del(e->vals[i]);
      e->vals[i] = val_copy(v);
      return;
    }
  }
  e->count++;

  e->vals = realloc(e->vals, sizeof(Val*) * e->count);
  e->vals[e->count - 1] = val_copy(v);

  e->syms = realloc(e->syms, sizeof(char*) * e->count);
  e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
  strcpy(e->syms[e->count - 1], k->sym);
}

void env_add_builtin(Env *e, char *name, BuiltIn func) {
  Val *k = val_sym(name);
  Val *v = val_func(func);
  env_put(e, k, v);
  val_del(k);
  val_del(v);
}

void env_add_builtins(Env *e) {
  env_add_builtin(e, "+", builtin_add);
  env_add_builtin(e, "-", builtin_sub);
  env_add_builtin(e, "*", builtin_mul);
  env_add_builtin(e, "/", builtin_div);
  env_add_builtin(e, "%", builtin_mod);
  env_add_builtin(e, "min", builtin_min);
  env_add_builtin(e, "max", builtin_max);

  env_add_builtin(e, "list", builtin_list);
  env_add_builtin(e, "head", builtin_head);
  env_add_builtin(e, "tail", builtin_tail);
  env_add_builtin(e, "eval", builtin_eval);
  env_add_builtin(e, "join", builtin_join);

  env_add_builtin(e, "def", builtin_def);
  env_add_builtin(e, "\\", builtin_lambda);
  env_add_builtin(e, ":=", builtin_assign);
}

Env *env_init(void) {
  Env *e = env_new();
  env_add_builtins(e);
  return e;
}

/*
 * Control functions
 */

void startup_info(void) {
  puts("its-lisp v0.1\nctrl-c to exit\n");
}

int should_exit(char *input) {
  int exit = strncmp(input, "exit", 4);
  int quit = strncmp(input, "quit", 4);
  return exit == 0 || quit == 0;
}

void process_input(Env *e, char *input, mpc_parser_t *lisp) {
  mpc_result_t r;
  if (mpc_parse("<stdin>", input, lisp, &r)) {
    // mpc_ast_print(r.output);
    Val *v = val_eval(e, val_read(r.output));
    val_println(v);
    val_del(v);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

int main(int argc, char **argv) {
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Symbol = mpc_new("symbol");
  mpc_parser_t *SExpr = mpc_new("sexpr");
  mpc_parser_t *QExpr = mpc_new("qexpr");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *ItsLisp = mpc_new("its_lisp");

  mpca_lang(
    MPCA_LANG_DEFAULT,
    "                                                         \
      number    : /-?[0-9]+/ ;                                \
      symbol    : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&%]+/ ;         \
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

  Env *env = env_init();

  while (1) {
    char *input = readline("its> ");
    if (should_exit(input)) break;
    add_history(input);
    process_input(env, input, ItsLisp);
  }

  env_del(env);

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

