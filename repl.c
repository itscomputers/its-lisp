/* repl.c */

#include "repl.h"

/*
 * Val builders
 */

Val *val_num(long n) {
  Val *v = malloc(sizeof(Val));
  v->type = VAL_NUM;
  v->num = n;
  return v;
}

Val *val_err(Err *err) {
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
 * Control functions
 */

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

