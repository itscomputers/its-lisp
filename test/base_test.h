/* test/base_test.h */

#define assert_eq_int(a, b, field) do { \
  long a_ = a; \
  long b_ = b; \
  if (a_ != b_) { \
    printf("\n  ***%s: expected %li to equal %li***", field, a_, b_); \
    return 0; \
  } \
} while (0)

#define assert_eq_str(a, b, field) do { \
  if (strcmp(a, b) != 0) { \
    printf("\n  ***%s: expected `%s` to equal `%s`***", field, a, b); \
    return 0; \
  } \
} while (0)

#define run_test(test) do { \
  int result = test(); \
  if (!result) { return result; } \
  printf(": pass\n"); \
} while (0)

#define assert_type(a, b) { assert_eq_int(a, b, "type"); }
#define assert_num(a, b) { assert_eq_int(a, b, "num"); }
#define assert_count(a, b) { assert_eq_int(a, b, "count"); }
#define assert_sym(a, b) { assert_eq_str(a, b, "sym"); }
#define assert_err_type(a, b) { assert_eq_int(a, b, "err->type"); }
#define assert_detail(a, b) { assert_eq_str(a, b, "err->det"); }

#define n(num) val_num(num)
#define s(sym) val_sym(sym)

#define begin_test { printf("%s", __func__); increment_tests_run; }

Val *build_sexpr(int arg_count, ...);
Val *build_qexpr(int arg_count, ...);

int error_tests(void);
int error_tests_run;

