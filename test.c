#define CHAR_IMPLEMENTATION
#define CHAR_STRIP_PREFIX
#include "char.h"
#include <stdio.h>

int main(void) {
  String str = string_from_cstr("Hello, world ! 🤡");
  String str2 = string_from_cstr(" <- This is the :clown: emoji");
  string_concat(&str, &str2);

  StringIter it = string_iter_begin(&str);
  {
    Char character = {0};
    size_t idx = it.index;
    while (string_iter_next(&it, &character)) {
      printf("%lu -> " StrFmt "\n", idx, CharArg(character));

      idx = it.index;
    }
  }
  string_iter_end(&it);

  printf("Text is: " StrFmt "\n", StrArg(str));
  return 0;
}
