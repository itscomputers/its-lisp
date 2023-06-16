/* env.c */

#include "repl.h"

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

