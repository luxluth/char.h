#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_DIR "build/"

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  mkdir_if_not_exists(BUILD_DIR);

  Cmd cmd = {0};
  cmd_append(&cmd, "clang", "-Wall", "-Wextra", "-o", BUILD_DIR "test",
             "test.c");
  cmd_run(&cmd);

  cmd_append(&cmd, BUILD_DIR "test");
  cmd_run(&cmd);

  return 0;
}
