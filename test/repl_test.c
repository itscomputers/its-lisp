#include <stdio.h>
#include "minunit.h"
#include "../repl.h"

#define assert_type(a, b) { mu_assert("incorrect type", a == b); }
#define assert_num(a, b) { mu_assert("incorrect num", a == b); }
#define assert_count(a, b) { mu_assert("incorrect count", a == b); }
#define assert_sym(a, b) { mu_assert("incorrect sym", strcmp(a, b) == 0); }
#define assert_err_det(a, b) { mu_assert("incorrect detail", strcmp(a, b) == 0); }

int tests_run = 0;

char *test_arithmetic(void) {
  printf("%s", __func__);
  //       (% (- (/ (+ (* 11 7) 2) 3) 4) 5)
  //    ~> (% (- (/ (+ 77 2) 3) 4) 5)
  //    ~> (% (- (/ 79 3) 4) 5)
  //    ~> (% (- 26 4) 5)
  //    ~> (% 22 5)
  //    ~> 2

  Val *seventy_seven = val_sexpr();
  val_append(seventy_seven, val_sym("*"));
  val_append(seventy_seven, val_num(11));
  val_append(seventy_seven, val_num(7));

  Val *seventy_nine = val_sexpr();
  val_append(seventy_nine, val_sym("+"));
  val_append(seventy_nine, seventy_seven);
  val_append(seventy_nine, val_num(2));

  Val *twenty_six = val_sexpr();
  val_append(twenty_six, val_sym("/"));
  val_append(twenty_six, seventy_nine);
  val_append(twenty_six, val_num(3));

  Val *twenty_two = val_sexpr();
  val_append(twenty_two, val_sym("-"));
  val_append(twenty_two, twenty_six);
  val_append(twenty_two, val_num(4));

  Val *expr = val_sexpr();
  val_append(expr, val_sym("%"));
  val_append(expr, twenty_two);
  val_append(expr, val_num(5));

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 2);

  val_del(result);
  env_del(env);

  return 0;
}

char *test_min(void) {
  printf("%s", __func__);
  Val *expr = val_sexpr();
  val_append(expr, val_sym("min"));
  val_append(expr, val_num(3));
  val_append(expr, val_num(2));
  val_append(expr, val_num(7));
  val_append(expr, val_num(5));

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 2);

  val_del(result);
  env_del(env);

  return 0;
}

char *test_max(void) {
  printf("%s", __func__);
  Val *expr = val_sexpr();
  val_append(expr, val_sym("max"));
  val_append(expr, val_num(3));
  val_append(expr, val_num(2));
  val_append(expr, val_num(7));
  val_append(expr, val_num(5));

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 7);

  val_del(result);
  env_del(env);

  return 0;
}

char *test_head(void) {
  printf("%s", __func__);
  Val *qexpr = val_qexpr();
  val_append(qexpr, val_num(2));
  val_append(qexpr, val_num(3));
  val_append(qexpr, val_num(5));
  val_append(qexpr, val_num(7));

  Val *head = val_sexpr();
  val_append(head, val_sym("head"));
  val_append(head, qexpr);

  Val *expr = val_sexpr();
  val_append(expr, val_sym("eval"));
  val_append(expr, head);

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 2);

  val_del(result);
  env_del(env);

  return 0;
}

char *test_tail(void) {
  printf("%s", __func__);
  Val *qexpr = val_qexpr();
  val_append(qexpr, val_num(2));
  val_append(qexpr, val_num(3));
  val_append(qexpr, val_num(5));
  val_append(qexpr, val_sym("x"));

  Val *expr = val_sexpr();
  val_append(expr, val_sym("tail"));
  val_append(expr, qexpr);

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_QEXPR);
  assert_count(result->count, 3);

  assert_type(result->cell[0]->type, VAL_NUM);
  assert_num(result->cell[0]->num, 3);

  assert_type(result->cell[1]->type, VAL_NUM);
  assert_num(result->cell[1]->num, 5);

  assert_type(result->cell[2]->type, VAL_SYM);
  assert_sym(result->cell[2]->sym, "x");

  val_del(result);
  env_del(env);

  return 0;
}

char *test_list(void) {
  printf("%s", __func__);
  Val *expr = val_sexpr();
  val_append(expr, val_sym("list"));
  val_append(expr, val_num(2));
  val_append(expr, val_num(3));
  val_append(expr, val_num(5));

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_QEXPR);
  assert_count(result->count, 3);

  assert_type(result->cell[0]->type, VAL_NUM);
  assert_num(result->cell[0]->num, 2);

  assert_type(result->cell[1]->type, VAL_NUM);
  assert_num(result->cell[1]->num, 3);

  assert_type(result->cell[2]->type, VAL_NUM);
  assert_num(result->cell[2]->num, 5);

  val_del(result);
  env_del(env);

  return 0;
}

char *test_join(void) {
  printf("%s", __func__);
  Val *first = val_qexpr();
  val_append(first, val_num(2));
  val_append(first, val_num(3));

  Val *second = val_qexpr();
  val_append(second, val_num(3));
  val_append(second, val_num(5));

  Val *expr = val_sexpr();
  val_append(expr, val_sym("join"));
  val_append(expr, first);
  val_append(expr, second);

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_QEXPR);
  assert_count(result->count, 4);

  assert_type(result->cell[0]->type, VAL_NUM);
  assert_num(result->cell[0]->num, 2);

  assert_type(result->cell[1]->type, VAL_NUM);
  assert_num(result->cell[1]->num, 3);

  assert_type(result->cell[2]->type, VAL_NUM);
  assert_num(result->cell[2]->num, 3);

  assert_type(result->cell[3]->type, VAL_NUM);
  assert_num(result->cell[3]->num, 5);

  val_del(result);
  env_del(env);

  return 0;
}

char *test_def(void) {
  printf("%s", __func__);
  Env *env = env_init();

  Val *x = val_qexpr();
  val_append(x, val_sym("x"));

  Val *def = val_sexpr();
  val_append(def, val_sym("def"));
  val_append(def, x);
  val_append(def, val_num(2));

  val_eval(env, def);

  Val *expr = val_sexpr();
  val_append(expr, val_sym("+"));
  val_append(expr, val_sym("x"));
  val_append(expr, val_num(3));

  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 5);

  val_del(def);
  val_del(result);
  env_del(env);

  return 0;
}

char *test_lambda(void) {
  printf("%s", __func__);
  Val *args = val_qexpr();
  val_append(args, val_sym("x"));

  Val *body = val_qexpr();
  val_append(body, val_sym("*"));
  val_append(body, val_sym("x"));
  val_append(body, val_sym("x"));

  Val *lambda = val_sexpr();
  val_append(lambda, val_sym("\\"));
  val_append(lambda, args);
  val_append(lambda, body);

  Val *expr = val_sexpr();
  val_append(expr, lambda);
  val_append(expr, val_num(3));

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 9);

  val_del(result);
  env_del(env);

  return 0;
}

char *test_division_by_zero(char *sym) {
  Val *expr = val_sexpr();
  val_append(expr, val_sym(sym));
  val_append(expr, val_num(3));
  val_append(expr, val_num(0));

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_count(result->err->type, ERR_ARITHMETIC);
  assert_err_det(result->err->det, "division by zero");

  val_del(result);
  env_del(env);

  return 0;
}

char *test_div_by_zero(void) {
  printf("%s", __func__);
  return test_division_by_zero("/");
}

char *test_mod_by_zero(void) {
  printf("%s", __func__);
  return test_division_by_zero("%");
}

char *test_builtin_op_wrong_type(Env *env, Val *val, char *det) {
  Val *expr = val_sexpr();
  val_append(expr, val_sym("+"));
  val_append(expr, val_num(2));
  val_append(expr, val);

  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_count(result->err->type, ERR_TYPE);
  assert_err_det(result->err->det, det);

  val_del(result);

  return 0;
}

char *test_builtin_op_with_sym_error(void) {
  printf("%s", __func__);
  Env *env = env_init();
  Val *val = val_sym("x");
  env_put(env, val, val);
  char *det = "expected number at index 1, got symbol";
  char *result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

char *test_builtin_op_with_func_error(void) {
  printf("%s", __func__);
  Env *env = env_init();
  Val *val = val_sym("min");
  char *det = "expected number at index 1, got function";
  char *result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

char *test_builtin_op_with_qexpr_error(void) {
  printf("%s", __func__);
  Env *env = env_init();
  Val *val = val_qexpr();
  char *det = "expected number at index 1, got q-expression";
  char *result = test_builtin_op_wrong_type(env, val, det);
  env_del(env);
  return result;
}

char *test_eval_sexpr_func_error(void) {
  printf("%s", __func__);
  Env *env = env_init();

  Val *expr = val_sexpr();
  val_append(expr, val_num(2));
  val_append(expr, val_num(3));

  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_ERR);
  assert_count(result->err->type, ERR_TYPE);
  assert_err_det(result->err->det, "expected function at index 0, got number");

  val_del(result);
  env_del(env);

  return 0;
}

char *all_tests(void) {
  mu_run_test(test_arithmetic);
  mu_run_test(test_min);
  mu_run_test(test_max);
  mu_run_test(test_head);
  mu_run_test(test_tail);
  mu_run_test(test_list);
  mu_run_test(test_join);
  mu_run_test(test_def);
  mu_run_test(test_lambda);
  mu_run_test(test_div_by_zero);
  mu_run_test(test_mod_by_zero);
  mu_run_test(test_builtin_op_with_sym_error);
  mu_run_test(test_builtin_op_with_func_error);
  mu_run_test(test_builtin_op_with_qexpr_error);
  mu_run_test(test_eval_sexpr_func_error);

  return 0;
}

int main(int argc, char **argv) {
  char *result = all_tests();
  if (result == 0) {
    printf("\nall tests passed!!\n");
  }
  printf("\ntests run: %i\n", tests_run);
  return 0;
}
