#include <stdio.h>

#include "../repl.h"
#include "base_test.h"

#define increment_tests_run { error_tests_run++; }

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

  return 1;
}

int test_div_by_zero(void) {
  begin_test;
  return test_division_by_zero("/");
}

int test_mod_by_zero(void) {
  begin_test;
  return test_division_by_zero("%");
}

int test_builtin_op_wrong_type(Env *env, Val *val, char *det) {
  Val *expr = build_sexpr(3, s("+"), n(2), val);
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, det);

  val_del(result);

  return 1;
}

int test_builtin_op_with_sym_error(void) {
  begin_test;
  Env *env = env_init();
  Val *val = s("x");
  env_put(env, val, val);
  char *det = "expected number at index 1, got symbol";
  int result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

int test_builtin_op_with_func_error(void) {
  begin_test;
  Env *env = env_init();
  Val *val = s("min");
  char *det = "expected number at index 1, got function";
  int result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

int test_builtin_op_with_qexpr_error(void) {
  begin_test;
  Env *env = env_init();
  Val *val = val_qexpr();
  char *det = "expected number at index 1, got q-expression";
  int result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

int test_eval_sexpr_func_error(void) {
  begin_test;
  Env *env = env_init();
  Val *expr = build_sexpr(2, n(2), n(3));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, "expected function at index 0, got number");

  val_del(result);
  env_del(env);

  return 1;
}

int test_too_many_args(char *sym) {
  Env *env = env_init();
  Val *expr = build_sexpr(3, s(sym), build_qexpr(1, n(2)), n(3));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_ARG);
  assert_detail(result->err->det, "expected 1 arguments, got 2");

  val_del(result);
  env_del(env);

  return 1;
}

int test_head_too_many_args(void) {
  begin_test;
  return test_too_many_args("head");
}

int test_tail_too_many_args(void) {
  begin_test;
  return test_too_many_args("tail");
}

int test_eval_too_many_args(void) {
  begin_test;
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

  return 1;
}

int test_head_wrong_type(void) {
  begin_test;
  return test_expected_qexpr("head");
}

int test_tail_wrong_type(void) {
  begin_test;
  return test_expected_qexpr("tail");
}

int test_eval_wrong_type(void) {
  begin_test;
  return test_expected_qexpr("eval");
}

int test_empty_qexpr(char *sym) {
  Env *env = env_init();
  Val *expr = build_sexpr(2, s(sym), val_qexpr());
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_type(result->err->type, ERR_ARG);
  assert_detail(result->err->det, "expected some arguments at index 0, got 0");

  val_del(result);
  env_del(env);

  return 1;
}

int test_head_empty(void) {
  begin_test;
  return test_empty_qexpr("head");
}

int test_tail_empty(void) {
  begin_test;
  return test_empty_qexpr("tail");
}

int test_join_wrong_type_0(void) {
  begin_test;
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

  return 1;
}

int test_join_wrong_type_1(void) {
  begin_test;
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

  return 1;
}

int test_join_wrong_type_2(void) {
  begin_test;
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

  return 1;
}

int test_def_arg0_wrong_type(Env *env, Val *val, char *detail) {
  Val *expr = build_sexpr(2, s("def"), val);
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, detail);

  val_del(result);
  env_del(env);

  return 1;
}

int test_def_arg0_number(void) {
  begin_test;
  Env *env = env_init();
  return test_def_arg0_wrong_type(
    env,
    n(2),
    "expected q-expression at index 0, got number"
  );
}

int test_def_arg0_symbol(void) {
  begin_test;
  Env *env = env_init();
  Val *val = s("x");
  env_put(env, val, val);
  return test_def_arg0_wrong_type(
    env,
    val,
    "expected q-expression at index 0, got symbol"
  );
}

int test_def_arg0_function(void) {
  begin_test;
  Env *env = env_init();
  return test_def_arg0_wrong_type(
    env,
    s("+"),
    "expected q-expression at index 0, got function"
  );
}

int test_def_arg0_sexpr(void) {
  begin_test;
  Env *env = env_init();
  return test_def_arg0_wrong_type(
    env,
    val_sexpr(),
    "expected q-expression at index 0, got s-expression"
  );
}

int test_def_arg1_wrong_type(Val *val, char *detail) {
  Env *env = env_init();
  Val *expr = build_sexpr(2, s("def"), build_qexpr(3, s("x"), s("y"), val));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, detail);

  val_del(result);
  env_del(env);

  return 1;
}

int test_def_arg1_number(void) {
  begin_test;
  return test_def_arg1_wrong_type(
    n(2),
    "expected symbol at index 2, got number"
  );
}

int test_def_arg1_sexpr(void) {
  begin_test;
  return test_def_arg1_wrong_type(
    build_sexpr(3, s("+"), n(2), n(3)),
    "expected symbol at index 2, got s-expression"
  );
}

int test_def_arg1_qexpr(void) {
  begin_test;
  return test_def_arg1_wrong_type(
    build_qexpr(3, s("y"), n(2), n(3)),
    "expected symbol at index 2, got q-expression"
  );
}

int test_def_wrong_arg_count(void) {
  begin_test;
  Env *env = env_init();
  Val *expr = build_sexpr(3,
    s("def"),
    build_qexpr(2, s("x"), s("y")),
    n(2)
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_ARG);
  assert_detail(result->err->det, "expected 1 arguments, got 2");

  val_del(result);
  env_del(env);

  return 1;
}

int test_lambda_wrong_arg_count_1(void) {
  begin_test;
  Env *env = env_init();
  Val *expr = build_sexpr(2,
    s("\\"),
    build_qexpr(2, s("x"), s("y"))
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_ARG);
  assert_detail(result->err->det, "expected 2 arguments, got 1");

  val_del(result);
  env_del(env);

  return 1;
}

int test_lambda_wrong_arg_count_3(void) {
  begin_test;
  Env *env = env_init();
  Val *expr = build_sexpr(4,
    s("\\"),
    build_qexpr(2, s("x"), s("y")),
    build_qexpr(3, s("+"), s("x"), s("y")),
    build_qexpr(1, s("x"))
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_ARG);
  assert_detail(result->err->det, "expected 2 arguments, got 3");

  val_del(result);
  env_del(env);

  return 1;
}

int test_lambda_wrong_arg0_type(Env *env, Val *val, char *detail) {
  Val *expr = build_sexpr(3,
    s("\\"),
    val,
    build_qexpr(3, s("+"), s("x"), s("y"))
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, detail);

  val_del(result);
  env_del(env);

  return 1;
}

int test_lambda_arg0_number(void) {
  begin_test;
  Env *env = env_init();
  return test_lambda_wrong_arg0_type(
    env,
    n(2),
    "expected q-expression at index 0, got number"
  );
}

int test_lambda_arg0_symbol(void) {
  begin_test;
  Env *env = env_init();
  Val *sym = s("x");
  env_put(env, sym, sym);
  return test_lambda_wrong_arg0_type(
    env,
    sym,
    "expected q-expression at index 0, got symbol"
  );
}

int test_lambda_arg0_function(void) {
  begin_test;
  Env *env = env_init();
  return test_lambda_wrong_arg0_type(
    env,
    s("+"),
    "expected q-expression at index 0, got function"
  );
}

int test_lambda_arg0_sexpr(void) {
  begin_test;
  Env *env = env_init();
  return test_lambda_wrong_arg0_type(
    env,
    val_sexpr(),
    "expected q-expression at index 0, got s-expression"
  );
}

int test_lambda_wrong_arg1_type(Env *env, Val *val, char *detail) {
  Val *expr = build_sexpr(3,
    s("\\"),
    build_qexpr(2, s("x"), s("y")),
    val
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, detail);

  val_del(result);
  env_del(env);

  return 1;
}

int test_lambda_arg1_number(void) {
  begin_test;
  Env *env = env_init();
  return test_lambda_wrong_arg1_type(
    env,
    n(2),
    "expected q-expression at index 1, got number"
  );
}

int test_lambda_arg1_symbol(void) {
  begin_test;
  Env *env = env_init();
  Val *sym = s("x");
  env_put(env, sym, sym);
  return test_lambda_wrong_arg1_type(
    env,
    sym,
    "expected q-expression at index 1, got symbol"
  );
}

int test_lambda_arg1_function(void) {
  begin_test;
  Env *env = env_init();
  return test_lambda_wrong_arg1_type(
    env,
    s("+"),
    "expected q-expression at index 1, got function"
  );
}

int test_lambda_arg1_sexpr(void) {
  begin_test;
  Env *env = env_init();
  return test_lambda_wrong_arg1_type(
    env,
    val_sexpr(),
    "expected q-expression at index 1, got s-expression"
  );
}

int test_lambda_wrong_arg0_inner_type(Val *val, char *detail) {
  Env *env = env_init();
  Val *expr = build_sexpr(3,
    s("\\"),
    build_qexpr(2, s("x"), val),
    build_qexpr(3, s("+"), s("x"), s("y"))
  );
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_err_type(result->err->type, ERR_TYPE);
  assert_detail(result->err->det, detail);

  val_del(result);
  env_del(env);

  return 1;
}

int test_lambda_arg0_inner_number(void) {
  begin_test;
  return test_lambda_wrong_arg0_inner_type(
    n(2),
    "expected symbol at index 1, got number"
  );
}

int test_lambda_arg0_inner_sexpr(void) {
  begin_test;
  return test_lambda_wrong_arg0_inner_type(
    val_sexpr(),
    "expected symbol at index 1, got s-expression"
  );
}

int test_lambda_arg0_inner_qexpr(void) {
  begin_test;
  return test_lambda_wrong_arg0_inner_type(
    val_qexpr(),
    "expected symbol at index 1, got q-expression"
  );
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

  run_test(test_def_arg0_number);
  run_test(test_def_arg0_symbol);
  run_test(test_def_arg0_function);
  run_test(test_def_arg0_sexpr);
  run_test(test_def_arg1_number);
  run_test(test_def_arg1_sexpr);
  run_test(test_def_arg1_qexpr);
  run_test(test_def_wrong_arg_count);
  run_test(test_def_wrong_arg_count);

  run_test(test_lambda_wrong_arg_count_1);
  run_test(test_lambda_wrong_arg_count_3);
  run_test(test_lambda_arg0_number);
  run_test(test_lambda_arg0_symbol);
  run_test(test_lambda_arg0_function);
  run_test(test_lambda_arg0_sexpr);
  run_test(test_lambda_arg1_number);
  run_test(test_lambda_arg1_symbol);
  run_test(test_lambda_arg1_function);
  run_test(test_lambda_arg1_sexpr);
  run_test(test_lambda_arg0_inner_number);
  run_test(test_lambda_arg0_inner_sexpr);
  run_test(test_lambda_arg0_inner_qexpr);

  return 1;
}

