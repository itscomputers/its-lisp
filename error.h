/* error.h */

typedef struct Err {
  int type;
  char *name;
  char *det;
} Err;

enum {
  ERR_ARG,
  ERR_TYPE,
  ERR_ARITHMETIC,
  ERR_VALUE,
  ERR_STANDARD,
};

char *err_name(int e);
Err *err_new(int type, char *fmt, ...);
Err *err_copy(Err *e);
void err_del(Err *e);

Err *err_parse_number(char *given);
Err *err_unbound_symbol(char *given);

Err *err_arg_type(char *expected, char *given);
Err *err_cell_arg_type(int index, char *expected, char *given);

Err *err_empty_args(void);
Err *err_arg_count(int expected, int given);
Err *err_empty_cell_args(int index);
Err *err_cell_arg_count(int index, int expected, int given);
