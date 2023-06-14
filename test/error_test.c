#include <stdio.h>

#include "../repl.h"
#include "base_test.h"

#define begin_test() { printf("%s", __func__); error_tests_run++; }

int error_tests_run = 0;

int test_division_by_zero(char *sym) {
  Env *env = env_init();
  Val *expr = build_sexpr(3, s(sym), n(3), n(0));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_ARITHMETIC);
  assert_detail(result->err->det, "division by zero");

  val_del(result);
  env_del(env);

  return 0;
}

int test_div_by_zero(void) {
  begin_test();
  return test_division_by_zero("/");
}

int test_mod_by_zero(void) {
  begin_test();
  return test_division_by_zero("%");
}

int test_builtin_op_wrong_type(Env *env, Val *val, char *det) {
  Val *expr = build_sexpr(3, s("+"), n(2), val);
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, det);

  val_del(result);

  return 0;
}

int test_builtin_op_with_sym_error(void) {
  begin_test();
  Env *env = env_init();
  Val *val = val_sym("x");
  env_put(env, val, val);
  char *det = "expected number at index 1, got symbol";
  int result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

int test_builtin_op_with_func_error(void) {
  begin_test();
  Env *env = env_init();
  Val *val = val_sym("min");
  char *det = "expected number at index 1, got function";
  int result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

int test_builtin_op_with_qexpr_error(void) {
  begin_test();
  Env *env = env_init();
  Val *val = val_qexpr();
  char *det = "expected number at index 1, got q-expression";
  int result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

int test_eval_sexpr_func_error(void) {
  begin_test();
  Env *env = env_init();
  Val *expr = build_sexpr(2, n(2), n(3));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, "expected function at index 0, got number");

  val_del(result);
  env_del(env);

  return 0;
}

int test_too_many_args(char *sym) {
  Env *env = env_init();
  Val *expr = build_sexpr(3, s(sym), build_qexpr(1, n(2)), n(3));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_ARG);
  assert_detail(result->err->det, "expected 1 arguments, got 0");

  val_del(result);
  env_del(env);

  return 0;
}

int test_head_too_many_args(void) {
  begin_test();
  return test_too_many_args("head");
}

int test_tail_too_many_args(void) {
  begin_test();
  return test_too_many_args("tail");
}

int test_eval_too_many_args(void) {
  begin_test();
  return test_too_many_args("eval");
}

int test_expected_qexpr(char *sym) {
  Env *env = env_init();
  Val *expr = build_sexpr(2, s(sym), n(3));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, "expected q-expression at index 0, got number");

  val_del(result);
  env_del(env);

  return 0;
}

int test_head_wrong_type(void) {
  begin_test();
  return test_expected_qexpr("head");
}

int test_tail_wrong_type(void) {
  begin_test();
  return test_expected_qexpr("tail");
}

int test_eval_wrong_type(void) {
  begin_test();
  return test_expected_qexpr("eval");
}

int test_empty_qexpr(char *sym) {
  Env *env = env_init();
  Val *expr = build_sexpr(2, s(sym), val_qexpr());
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_type(result->err->type, ERR_ARG);
  assert_detail(result->err->det, "expected arguments at index 0, got 0");

  val_del(result);
  env_del(env);

  return 0;
}

int test_head_empty(void) {
  begin_test();
  return test_empty_qexpr("head");
}

int test_tail_empty(void) {
  begin_test();
  return test_empty_qexpr("tail");
}

int test_join_wrong_type_0(void) {
  begin_test();
  Env *env = env_init();
  Val *expr = build_sexpr(4,
    s("join"),
    n(5),
    build_qexpr(1, n(2)),
    build_qexpr(1, n(2))
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, "expected q-expression at index 0, got number");

  val_del(result);
  env_del(env);

  return 0;
}

int test_join_wrong_type_1(void) {
  begin_test();
  Env *env = env_init();
  Val *expr = build_sexpr(4,
    s("join"),
    build_qexpr(1, n(2)),
    n(5),
    build_qexpr(1, n(2))
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, "expected q-expression at index 1, got number");

  val_del(result);
  env_del(env);

  return 0;
}

int test_join_wrong_type_2(void) {
  begin_test();
  Env *env = env_init();
  Val *expr = build_sexpr(4,
    s("join"),
    build_qexpr(1, n(2)),
    build_qexpr(1, n(2)),
    n(5)
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, "expected q-expression at index 2, got number");

  val_del(result);
  env_del(env);

  return 0;
}

int error_tests(void) {
  run_test(test_div_by_zero);
  run_test(test_mod_by_zero);

  run_test(test_builtin_op_with_sym_error);
  run_test(test_builtin_op_with_func_error);
  run_test(test_builtin_op_with_qexpr_error);

  run_test(test_eval_sexpr_func_error);

  run_test(test_head_too_many_args);
  run_test(test_head_wrong_type);
  run_test(test_head_empty);

  run_test(test_tail_too_many_args);
  run_test(test_tail_wrong_type);
  run_test(test_tail_empty);

  run_test(test_eval_too_many_args);
  run_test(test_eval_wrong_type);

  run_test(test_join_wrong_type_0);
  run_test(test_join_wrong_type_1);
  run_test(test_join_wrong_type_2);

  return 0;
}

