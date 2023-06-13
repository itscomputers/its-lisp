/* test/minunit.h */
#define mu_assert(msg, test) do { \
  if (!(test)) return msg; \
} while (0)
#define mu_run_test(test) do { \
  char *msg = test(); \
  tests_run++; \
  if (msg) { \
    printf(": %s\n", msg); \
    return msg; \
  } else { \
    printf(": pass\n"); \
  } \
} while (0)
extern int tests_run;
