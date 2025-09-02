#define CHAR_IMPLEMENTATION
#define CHAR_STRIP_PREFIX
#include "char.h"
#include <stdio.h>

void debug_string(const Char_String *s) {
  fprintf(stderr,
          "String{.len = %lu, .count = %lu, .capacity = %lu, .items = \"" StrFmt
          "\"}\n",
          s->len, s->count, s->capacity, StrArg(*s));
}

int main(void) {
  String str = S("Hello, world ! 🤡");
  String str2 = S(" <- This is the :clown: emoji");
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

  debug_string(&str);
  string_insert(&str, S("🎈"), 15);
  debug_string(&str);

  String empty_str = {0};
  debug_string(&empty_str);
  string_insert(&empty_str, S("♥️"), 0);
  printf("From empty text to: " StrFmt "\n", StrArg(empty_str));
  string_insert(&empty_str, S(" - I think I love you"), empty_str.len);
  debug_string(&empty_str);

  return 0;
}
