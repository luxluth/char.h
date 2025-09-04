#define CHAR_IMPLEMENTATION
#define CHAR_STRIP_PREFIX
#include "char.h"
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <stdio.h>

void debug_string(const Char_String *s) {
  printf("String{.len = %lu, .count = %lu, .capacity = %lu, .items = \"" StrFmt
         "\"}\n",
         s->len, s->count, s->capacity, StrArg(*s));
}

int main(void) {
  Nob_String_Builder sb = {0};
  nob_read_entire_file("texts/victor_hugo_full.txt", &sb);
  String victor_text = string_from_raw_data((uint8_t *)sb.items, sb.count);
  debug_string(&victor_text);

  Nob_String_Builder sb1 = {0};
  nob_read_entire_file("texts/demo_utf8.txt", &sb1);
  String utf8_text = string_from_raw_data((uint8_t *)sb1.items, sb1.count);
  debug_string(&utf8_text);

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

  String t_remove = S("Too much 'a' in anaanas🍍");
  debug_string(&t_remove);
  string_remove(&t_remove, 19);
  debug_string(&t_remove);
  string_remove(&t_remove, t_remove.len - 1);
  debug_string(&t_remove);

  String sub_of_it = S("This Part and This Other One");
  debug_string(&sub_of_it);
  String first_out_sub = {0};
  string_substring(&sub_of_it, .start = 0, .len = 9, .out = &first_out_sub);
  debug_string(&first_out_sub);

  String end_out_sub = {0};
  string_substring(&sub_of_it, .start = sub_of_it.len - 1, .len = 14,
                   .out = &end_out_sub, .reverse = true);
  debug_string(&end_out_sub);

  int order = string_compare_codepoints(&victor_text, &victor_text);
  printf("order = %d == 0\n", order);

  if (string_equals(&utf8_text, &utf8_text)) {
    printf("equals :: ok\n");
  } else {
    printf("equals :: fail\n");
    return 1;
  }

  if (string_starts_with(&sub_of_it, &first_out_sub)) {
    printf("starts_with :: ok\n");
  } else {
    printf("starts_with :: fail\n");
  }

  if (string_ends_with(&sub_of_it, &end_out_sub)) {
    printf("ends_with :: ok\n");
  } else {
    printf("ends_with :: fail\n");
  }

  return 0;
}
