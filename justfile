deps := "env.c error.c eval.c mpc.c"
tests := "test/repl_test.c test/error_test.c test/base_test.c"

compile:
  gcc -o repl -Wall -ledit repl.c {{deps}}

@run:
  ./repl

debug:
  gcc -o repl -Wall -ledit -g repl.c {{deps}}
  lldb ./repl
  rm -rf repl.dSYM

@test: _test_setup && _test_cleanup
  ./test.out

@test_debug: _test_setup && _test_cleanup
  lldb ./test.out

@_test_setup:
  awk '{gsub(/int main/, "int main_tmp"); print}' repl.c > repl_tmp.c
  gcc -o test.out -Wall -ledit -g repl_tmp.c {{deps}} {{tests}}

@_test_cleanup:
  rm ./test.out
  rm -rf test.out.dSYM
  rm repl_tmp.c
