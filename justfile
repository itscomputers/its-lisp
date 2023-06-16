@compile:
  gcc -o repl -Wall -ledit \
    repl.c \
    error.c \
    mpc.c

@run:
  ./repl

@debug:
  gcc -o repl -Wall -ledit -g \
    repl.c \
    error.c \
    mpc.c
  lldb ./repl
  rm -rf repl.dSYM

@test: _test_setup && _test_cleanup
  ./test.out

@test_debug: _test_setup && _test_cleanup
  lldb ./test.out

@_test_setup:
  awk '{gsub(/int main/, "int main__"); print}' repl.c > repl__.c
  gcc -o test.out -Wall -ledit -g \
    repl__.c \
    error.c \
    mpc.c \
    test/repl_test.c \
    test/error_test.c \
    test/base_test.c

@_test_cleanup:
  rm ./test.out
  rm -rf test.out.dSYM
  rm repl__.c
