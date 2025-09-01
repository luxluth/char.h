#define CHAR_IMPLEMENTATION
#define CHAR_STRIP_PREFIX
#include "char.h"
#include <stdio.h>

int main(void) {
  String str = string_from_cstr("Hello, world ! 🤡");
  String str2 = string_from_cstr(" <- This is the :clown: emoji");
  string_concat(&str, &str2);

  printf("Text is: " StrFmt "\n", StrArg(str));
  return 0;
}
