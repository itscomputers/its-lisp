/* error.c */

#include "repl.h"

char *err_name(int e) {
  switch (e) {
    case ERR_ARG: return "ArgumentError";
    case ERR_TYPE: return "TypeError";
    case ERR_ARITHMETIC: return "ArithmeticError";
    case ERR_VALUE: return "ValueError";
    case ERR_STANDARD: return "StandardError";
  }
  return "Error";
}

Err *err_new(int type, char *fmt, ...) {
  Err *e = malloc(sizeof(Err));
  e->type = type;

  char *name = err_name(type);
  e->name = malloc(strlen(name) + 1);
  strcpy(e->name, name);

  va_list va;
  va_start(va, fmt);
  e->det = malloc(512);
  vsnprintf(e->det, 511, fmt, va);
  e->det = realloc(e->det, strlen(e->det) + 1);
  va_end(va);

  return e;
}

Err *err_copy(Err *e) {
  Err *c = malloc(sizeof(Err));
  c->type = e->type;
  c->name = malloc(strlen(e->name) + 1);
  strcpy(c->name, e->name);
  c->det = malloc(strlen(e->det) + 1);
  strcpy(c->det, e->det);
  return c;
}

void err_del(Err *e) {
  free(e->name);
  free(e->det);
  free(e);
}

Err *err_parse_number(char *given) {
  return err_new(
    ERR_VALUE,
    "unable to parse %s as number",
    given
  );
}

Err *err_unbound_symbol(char *given) {
  return err_new(
    ERR_VALUE,
    "unbound symbol: %s",
    given
  );
}

Err *err_arg_type(char *expected, char *given) {
  return err_new(
    ERR_TYPE,
    "expected %s, got %s",
    expected,
    given
  );
}

Err *err_cell_arg_type(int index, char *expected, char *given) {
  return err_new(
    ERR_TYPE,
    "expected %s at index %i, got %s",
    expected,
    index,
    given
  );
}

Err *err_empty_args(void) {
  return err_new(
    ERR_ARG,
    "expected some arguments, got 0"
  );
}

Err *err_arg_count(int expected, int given) {
  return err_new(
    ERR_ARG,
    "expected %i arguments, got %i",
    expected,
    given
  );
}

Err *err_empty_cell_args(int index) {
  return err_new(
    ERR_ARG,
    "expected some arguments at index %i, got 0",
    index
  );
}

Err *err_cell_arg_count(int index, int expected, int given) {
  return err_new(
    ERR_ARG,
    "expected %i arguments at index %i, got %i",
    expected,
    index,
    given
  );
}

