#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#ifndef CHARDEF
#define CHARDEF
#endif /* CHARDEF */

////////////////////////////////////////////////////////////// FROM nob.h

#ifdef __cplusplus
#define __CHAR_DECLTYPE_CAST(T) (decltype(T))
#else
#define __CHAR_DECLTYPE_CAST(T)
#endif /* __cplusplus */

#define CHAR_DA_INIT_CAP 256

#define char_da_reserve(da, expected_capacity)                                 \
  do {                                                                         \
    if ((expected_capacity) > (da)->capacity) {                                \
      if ((da)->capacity == 0) {                                               \
        (da)->capacity = CHAR_DA_INIT_CAP;                                     \
      }                                                                        \
      while ((expected_capacity) > (da)->capacity) {                           \
        (da)->capacity *= 2;                                                   \
      }                                                                        \
      (da)->items = __CHAR_DECLTYPE_CAST((da)->items)                          \
          realloc((da)->items, (da)->capacity * sizeof(*(da)->items));         \
      assert((da)->items != NULL && "Buy more RAM lol");                       \
    }                                                                          \
  } while (0)

#define char_da_append(da, item)                                               \
  do {                                                                         \
    char_da_reserve((da), (da)->count + 1);                                    \
    (da)->items[(da)->count++] = (item);                                       \
  } while (0)

#define char_da_free(da) free((da).items)

///////////////////////////////////////////////////////////

thread_local static uint8_t __char_temp[4];

typedef struct {
  uint8_t **items;
  size_t count;
  size_t capacity;

  size_t len;
} String;

CHARDEF String char_string_from(const char *chars);
CHARDEF void char_string_reset(String *str);
CHARDEF void char_string_dealloc(String str);
CHARDEF bool char_char2utf8(const char byte, uint8_t out[4]);

#ifdef CHAR_IMPLEMENTATION

CHARDEF bool char_char2utf8(const char byte, uint8_t out[4]) {}

CHARDEF String char_string_from(const char *chars) {
  String str = {0};
  size_t len = strlen(chars);
  size_t i = 0;

  while (i < len) {
    if (char_char2utf8(chars[i], __char_temp)) {
      char_da_append(&str, __char_temp);
    }
    i++;
  }

  return str;
}

CHARDEF void char_string_reset(String *str) {
  str->count = 0;
  str->len = 0;
}

CHARDEF void char_string_dealloc(String str) { char_da_free(str); }

#endif // CHAR_IMPLEMENTATION
