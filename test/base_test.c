/* test/base_test.c */
#include "../repl.h"

Val* build_sexpr(int arg_count, ...) {
  Val* sexpr = val_sexpr();

  va_list args;
  va_start(args, arg_count);
  for (int i = 0; i < arg_count; i++) {
    val_append(sexpr, va_arg(args, Val*));
  }
  va_end(args);

  return sexpr;
}

Val* build_qexpr(int arg_count, ...) {
  Val* qexpr = val_qexpr();

  va_list args;
  va_start(args, arg_count);
  for (int i = 0; i < arg_count; i++) {
    val_append(qexpr, va_arg(args, Val*));
  }
  va_end(args);

  return qexpr;
}
