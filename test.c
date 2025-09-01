#define CHAR_IMPLEMENTATION
#include "char.h"
#include <stdio.h>

int main(void) {
  String str = char_string_from("Hello, world !");
  printf("Text is: " StrFmt "\n", StrArg(str));
  return 0;
}
