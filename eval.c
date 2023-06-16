/* eval.c */

#include "repl.h"

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

