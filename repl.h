/* repl.h */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <editline/readline.h>
#include "mpc.h"

struct Val;
typedef struct Val Val;

struct Env;
typedef struct Env Env;

struct Err;
typedef struct Err Err;

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

struct Err {
  int type;
  char *name;
  char *det;
};

enum {
  VAL_NUM,
  VAL_ERR,
  VAL_SYM,
  VAL_FUNC,
  VAL_SEXPR,
  VAL_QEXPR
};

enum {
  ERR_ARG,
  ERR_TYPE,
  ERR_ARITHMETIC,
  ERR_VALUE,
  ERR_STANDARD,
};

Val *val_read(mpc_ast_t *t);
Val *val_call(Env *e, Val *f, Val *v);
Val *val_eval(Env *e, Val *v);
void val_del(Val *v);
void val_print(Val *v);
void val_println(Val *v);
Val *val_num(long n);
Val *val_sym(char *s);
Val *val_err(Err *err);
Val *val_sexpr(void);
Val *val_qexpr(void);
Val *val_append(Val *v, Val *c);

Env *env_new(void);
Env *env_init(void);
Env *env_copy(Env *e);
void env_del(Env *e);
Val *env_get(Env *e, Val *k);
void env_def(Env *e, Val *k, Val *v);
void env_put(Env *e, Val *k, Val *v);

char *err_name(int e);
Err *err_new(int type, char *fmt, ...);
Err *err_copy(Err *e);
void err_del(Err *e);
Err *err_parse_number(char *given);
Err *err_unbound_symbol(char *given);
Err *err_arg_type(char *expected, char *given);
Err *err_cell_arg_type(int index, char *expected, char *given);
Err *err_empty_args(void);
Err *err_arg_count(int expected, int given);
Err *err_empty_cell_args(int index);
Err *err_cell_arg_count(int index, int expected, int given);
