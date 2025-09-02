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

  Char temp_char = {0};
  string_char_at(&str, 0, &temp_char);
  printf("First character is `" StrFmt "`\n", CharArg(temp_char));

  string_char_at(&str, str.len - 1, &temp_char);
  printf("Last character is `" StrFmt "`\n", CharArg(temp_char));

  string_char_at(&str, 15, &temp_char);
  printf("The clown emoji is the 15th character `" StrFmt "`\n",
         CharArg(temp_char));

  printf("Text is: " StrFmt "\n", StrArg(str));
  return 0;
}
