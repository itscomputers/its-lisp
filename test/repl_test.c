#include "../repl.h"
#include "base_test.h"

#define increment_tests_run { tests_run++; }

int tests_run = 0;

int test_arithmetic(void) {
  begin_test;
  Env *env = env_init();

  Val *seventy_seven = build_sexpr(3, s("*"), n(11), n(7));
  Val *seventy_nine = build_sexpr(3, s("+"), seventy_seven, n(2));
  Val *twenty_six = build_sexpr(3, s("/"), seventy_nine, n(3));
  Val *twenty_two = build_sexpr(3, s("-"), twenty_six, n(4));
  Val *expr = build_sexpr(3, s("%"), twenty_two, n(5));

  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 2);

  val_del(result);
  env_del(env);

  return 1;
}

int test_min(void) {
  begin_test;
  Env *env = env_init();
  Val *expr = build_sexpr(5, s("min"), n(3), n(2), n(7), n(5));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 2);

  val_del(result);
  env_del(env);

  return 1;
}

int test_max(void) {
  begin_test;
  Env *env = env_init();
  Val *expr = build_sexpr(5, s("max"), n(3), n(2), n(7), n(5));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 7);

  val_del(result);
  env_del(env);

  return 1;
}

int test_head(void) {
  begin_test;

  Val *head = build_sexpr(2, s("head"), build_qexpr(4, n(2), n(3), n(5), s("x")));
  Val *expr = build_sexpr(2, s("eval"), head);

  Env *env = env_init();
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 2);

  val_del(result);
  env_del(env);

  return 1;
}

int test_tail(void) {
  begin_test;
  Env *env = env_init();
  Val *expr = build_sexpr(2, s("tail"), build_qexpr(4, n(2), n(3), n(5), s("x")));
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

  return 1;
}

int test_list(void) {
  begin_test;
  Env *env = env_init();

  Val *expr = build_sexpr(4, s("list"), n(2), n(3), n(5));
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

  return 1;
}

int test_join(void) {
  begin_test;
  Env *env = env_init();

  Val *expr = build_sexpr(3,
    s("join"),
    build_qexpr(2, n(2), n(3)),
    build_qexpr(2, n(3), n(5))
  );
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

  return 1;
}

int test_def(void) {
  begin_test;
  Env *env = env_init();

  Val *x = val_qexpr();
  val_append(x, val_sym("x"));

  Val *def = build_sexpr(3,
    s("def"),
    build_qexpr(1, s("x")),
    n(2)
  );
  val_eval(env, def);

  Val *expr = build_sexpr(3, s("+"), s("x"), n(3));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 5);

  val_del(def);
  val_del(result);
  env_del(env);

  return 1;
}

int test_lambda(void) {
  begin_test;
  Env *env = env_init();

  Val *lambda = build_sexpr(3,
    s("\\"),
    build_qexpr(1, s("x")),
    build_qexpr(3, s("*"), s("x"), s("x"))
  );

  Val *expr = build_sexpr(2, lambda, n(3));
  Val *result = val_eval(env, expr);

  assert_type(result->type, VAL_NUM);
  assert_num(result->num, 9);

  val_del(result);
  env_del(env);

  return 1;
}

int all_tests(void) {
  run_test(test_arithmetic);
  run_test(test_min);
  run_test(test_max);
  run_test(test_head);
  run_test(test_tail);
  run_test(test_list);
  run_test(test_join);
  run_test(test_def);
  run_test(test_lambda);

  error_tests();

  return 1;
}

int main(int argc, char **argv) {
  int result = all_tests();
  if (result == 1) {
    printf("\nall tests passed!!\n");
  }
  printf("\ntests run: %i\n", tests_run + error_tests_run);
  return !result;
}
