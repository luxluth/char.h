#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_DIR "build/"

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  mkdir_if_not_exists(BUILD_DIR);

  Cmd cmd = {0};
  cmd_append(&cmd, "clang", "-Wall", "-Wextra", "-O3", "-o", BUILD_DIR "test",
             "test.c");
  if (!cmd_run(&cmd))
    return 1;

  cmd_append(&cmd, BUILD_DIR "test");
  if (!cmd_run(&cmd))
    return 1;

  return 0;
}
