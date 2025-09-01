#ifndef CHAR_H_
#define CHAR_H_

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

#ifndef StrFmt
#define StrFmt "%.*s"
#endif /* StrFmt */
#ifndef StrArg
#define StrArg(s) (int)(s).count, (s).items
#endif /* StrArg */

thread_local static uint8_t __char_temp[4];

typedef struct {
  uint8_t *items;
  size_t count;
  size_t capacity;

  size_t len;
} String;

typedef enum {
  UTF8_INVALID = 0,
  UTF8_ONE_BYTE = 1,
  UTF8_TWO_BYTE = 2,
  UTF8_THREE_BYTE = 3,
  UTF8_FOUR_BYTE = 4,
} CHAR_UTF8_TYPE;

CHARDEF String char_string_from(const char *chars);
CHARDEF void char_string_reset(String *str);
CHARDEF void char_string_dealloc(String str);
CHARDEF CHAR_UTF8_TYPE char_parse_utf8_codepoint(const uint8_t *input,
                                                 size_t width, uint8_t out[4],
                                                 size_t *read);

#endif // CHAR_H_

// #define CHAR_IMPLEMENTATION
#ifdef CHAR_IMPLEMENTATION

static inline bool __char_is_cont(uint8_t b) {
  return (b & 0b11000000) == 0b10000000;
}

CHARDEF CHAR_UTF8_TYPE char_parse_utf8_codepoint(const uint8_t *input,
                                                 size_t width, uint8_t out[4],
                                                 size_t *read) {
  if (width == 0)
    return UTF8_INVALID;

  uint8_t b0 = input[0];
  size_t n = 0;

  if ((b0 & 0b10000000) == 0b00000000) { // 0xxxxxxx
    n = 1;
  } else if ((b0 & 0b11100000) == 0b11000000) { // 110xxxxx
    n = 2;
  } else if ((b0 & 0b11110000) == 0b11100000) { // 1110xxxx
    n = 3;
  } else if ((b0 & 0b11111000) == 0b11110000) { // 11110xxx
    n = 4;
  } else {
    return UTF8_INVALID;
  }
  if (width < n)
    return UTF8_INVALID;

  // Checking for continuation bytes
  for (size_t i = 1; i < n; i++) {
    if (!__char_is_cont(input[i]))
      return UTF8_INVALID;
  }

  // Decoding to a code point to validate range/overlongs
  uint32_t cp = 0;
  switch (n) {
  case 1: {
    cp = b0;
  } break;
  case 2: {
    cp = ((uint32_t)(b0 & 0x1Fu) << 6) | (uint32_t)(input[1] & 0x3Fu);
    if (cp < 0x80u)
      return UTF8_INVALID; // overlong
  } break;
  case 3: {
    cp = ((uint32_t)(b0 & 0x0Fu) << 12) | ((uint32_t)(input[1] & 0x3Fu) << 6) |
         (uint32_t)(input[2] & 0x3Fu);
    if (cp < 0x800u)
      return UTF8_INVALID; // overlong
    if (cp >= 0xD800u && cp <= 0xDFFFu)
      return UTF8_INVALID; // UTF-16 surrogates
  } break;
  case 4: {
    cp = ((uint32_t)(b0 & 0x07u) << 18) | ((uint32_t)(input[1] & 0x3Fu) << 12) |
         ((uint32_t)(input[2] & 0x3Fu) << 6) | (uint32_t)(input[3] & 0x3Fu);
    if (cp < 0x10000u || cp > 0x10FFFFu)
      return UTF8_INVALID; // overlong/out of range
  } break;
  }

  // Copy bytes out
  for (size_t i = 0; i < n; ++i)
    out[i] = input[i];
  if (read)
    *read = n;

  switch (n) {
  case 1:
    return UTF8_ONE_BYTE;
  case 2:
    return UTF8_TWO_BYTE;
  case 3:
    return UTF8_THREE_BYTE;
  case 4:
    return UTF8_FOUR_BYTE;
  default:
    return UTF8_INVALID;
  }
}

CHARDEF String char_string_from(const char *chars) {
  String str = {0};
  size_t len = strlen(chars);
  const uint8_t *p = (const uint8_t *)chars;
  size_t i = 0;

  while (i < len) {
    size_t read = 0;
    CHAR_UTF8_TYPE kind =
        char_parse_utf8_codepoint(p + i, len - i, __char_temp, &read);

    if (kind == UTF8_INVALID) {
      break;
    }

    for (size_t j = 0; j < read; ++j)
      char_da_append(&str, __char_temp[j]);

    str.len += 1;
    i += read;
  }

  return str;
}

CHARDEF void char_string_reset(String *str) {
  str->count = 0;
  str->len = 0;
}

CHARDEF void char_string_dealloc(String str) { char_da_free(str); }

#endif // CHAR_IMPLEMENTATION
