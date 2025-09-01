#ifndef CHAR_H_
#define CHAR_H_

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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

#define char_da_append_many(da, new_items, new_items_count)                    \
  do {                                                                         \
    char_da_reserve((da), (da)->count + (new_items_count));                    \
    memcpy((da)->items + (da)->count, (new_items),                             \
           (new_items_count) * sizeof(*(da)->items));                          \
    (da)->count += (new_items_count);                                          \
  } while (0)

#define char_da_free(da) free((da)->items)

#define CHAR_TODO(message)                                                     \
  do {                                                                         \
    fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message);         \
    abort();                                                                   \
  } while (0)
#define CHAR_UNREACHABLE(message)                                              \
  do {                                                                         \
    fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message);  \
    abort();                                                                   \
  } while (0)

///////////////////////////////////////////////////////////

#ifndef StrFmt
#define StrFmt "%.*s"
#endif /* StrFmt */
#ifndef StrArg
#define StrArg(s) (int)(s).count, (s).items
#endif /* StrArg */

thread_local static uint8_t __char_temp[4];

// It is a dynamic array of utf-8 codepoints
// * count - represent byte size
// * len   - represent the string length
//
// This structure can be printed as follow:
//     printf(StrFmt"\n", StrArg(str));
typedef struct {
  uint8_t *items;
  size_t count;
  size_t capacity;

  size_t len;
} Char_String;

typedef enum {
  CHAR_UTF8_INVALID = 0,
  CHAR_UTF8_ONE_BYTE = 1,
  CHAR_UTF8_TWO_BYTE = 2,
  CHAR_UTF8_THREE_BYTE = 3,
  CHAR_UTF8_FOUR_BYTE = 4,
} CHAR_UTF8_TYPE;

// Create a string structure from the default c string
CHARDEF Char_String char_string_from_cstr(const char *chars);

// Reset the string’s length to zero without freeing capacity.
CHARDEF void char_string_clear(Char_String *str);

// Deallocates the string object. After calling this function,
// the string object should not be used anymore
CHARDEF void char_string_dealloc(Char_String *str);

// Make a deep copy of the provided string
CHARDEF Char_String char_string_clone(const Char_String *s);

// Ensure capacity is at least `cap` bytes.
CHARDEF void char_string_reserve(Char_String *s, size_t cap);

// Allocate and return a null-terminated C string copy.
// - Caller must free the returned pointer.
CHARDEF char *char_string_to_cstr(const Char_String *s);

// Append raw UTF-8 sequence(s).
CHARDEF bool char_string_push_utf8(Char_String *str, const uint8_t *p,
                                   size_t len);

// Append the contents of `other` to `s`.
CHARDEF bool char_string_concat(Char_String *s, const Char_String *other);

#endif // CHAR_H_

/* #define CHAR_IMPLEMENTATION */
// use #define CHAR_IMPLEMENTATION
// to include also the library implementation
#ifdef CHAR_IMPLEMENTATION

static inline bool __char_is_cont(uint8_t b) {
  return (b & 0b11000000) == 0b10000000;
}

CHARDEF bool __char_utf8_validate(const uint8_t *p, size_t width,
                                  size_t *read) {
  if (width == 0)
    return false;

  uint8_t b0 = p[0];
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
    return false;
  }
  if (width < n)
    return false;

  // Checking for continuation bytes
  for (size_t i = 1; i < n; i++) {
    if (!__char_is_cont(p[i]))
      return false;
  }

  // Decoding to a code point to validate range/overlongs
  uint32_t cp = 0;
  switch (n) {
  case 1: {
    cp = b0;
  } break;
  case 2: {
    cp = ((uint32_t)(b0 & 0x1Fu) << 6) | (uint32_t)(p[1] & 0x3Fu);
    if (cp < 0x80u)
      return false; // overlong
  } break;
  case 3: {
    cp = ((uint32_t)(b0 & 0x0Fu) << 12) | ((uint32_t)(p[1] & 0x3Fu) << 6) |
         (uint32_t)(p[2] & 0x3Fu);
    if (cp < 0x800u)
      return false; // overlong
    if (cp >= 0xD800u && cp <= 0xDFFFu)
      return false; // UTF-16 surrogates
  } break;
  case 4: {
    cp = ((uint32_t)(b0 & 0x07u) << 18) | ((uint32_t)(p[1] & 0x3Fu) << 12) |
         ((uint32_t)(p[2] & 0x3Fu) << 6) | (uint32_t)(p[3] & 0x3Fu);
    if (cp < 0x10000u || cp > 0x10FFFFu)
      return false; // overlong/out of range
  } break;
  }

  if (read)
    *read = n;

  return true;
}

CHARDEF bool __char_utf8_encode(uint32_t cp, uint8_t out[4], size_t *w) {
  (void)cp;
  (void)out;
  (void)w;
  CHAR_TODO("__char_utf8_encode");
}

CHARDEF CHAR_UTF8_TYPE __char_utf8_decode_next(const uint8_t *p, size_t width,
                                               uint8_t out[4], size_t *read) {
  if (__char_utf8_validate(p, width, read)) {
    size_t n = *read;
    // Copy bytes out
    for (size_t i = 0; i < n; ++i)
      out[i] = p[i];

    switch (n) {
    case 1:
      return CHAR_UTF8_ONE_BYTE;
    case 2:
      return CHAR_UTF8_TWO_BYTE;
    case 3:
      return CHAR_UTF8_THREE_BYTE;
    case 4:
      return CHAR_UTF8_FOUR_BYTE;
    default:
      CHAR_UNREACHABLE("Should be a valid utf-8");
    }
  } else {
    return CHAR_UTF8_INVALID;
  }
}

CHARDEF Char_String char_string_from_cstr(const char *chars) {
  Char_String str = {0};
  size_t len = strlen(chars);
  const uint8_t *p = (const uint8_t *)chars;
  char_string_push_utf8(&str, p, len);

  return str;
}

CHARDEF void char_string_clear(Char_String *str) {
  str->count = 0;
  str->len = 0;
}

CHARDEF void char_string_dealloc(Char_String *str) {
  char_da_free(str);
  str->len = 0;
}

CHARDEF Char_String char_string_clone(const Char_String *s) {
  Char_String new_string = {0};
  char_da_append_many(&new_string, s->items, s->count);
  new_string.len = s->len;
  return new_string;
}

CHARDEF void char_string_reserve(Char_String *s, size_t cap) {
  char_da_reserve(s, cap);
}

CHARDEF char *char_string_to_cstr(const Char_String *s) {
  Char_String cloned_s = char_string_clone(s);
  char_da_append(&cloned_s, '\0');
  return (char *)cloned_s.items;
}

CHARDEF bool char_string_push_utf8(Char_String *str, const uint8_t *p,
                                   size_t len) {
  size_t i = 0;
  while (i < len) {
    size_t read = 0;
    CHAR_UTF8_TYPE kind =
        __char_utf8_decode_next(p + i, len - i, __char_temp, &read);

    if (kind == CHAR_UTF8_INVALID) {
      return false;
    }

    for (size_t j = 0; j < read; ++j)
      char_da_append(str, __char_temp[j]);

    str->len += 1;
    i += read;
  }

  return true;
}

CHARDEF bool char_string_concat(Char_String *s, const Char_String *other) {
  return char_string_push_utf8(s, other->items, other->count);
}

#endif // CHAR_IMPLEMENTATION

#ifndef CHAR_STRIP_PREFIX_GUARD_
#define CHAR_STRIP_PREFIX_GUARD_

#ifdef CHAR_STRIP_PREFIX
#define String Char_String
#define string_from_cstr char_string_from_cstr
#define string_clear char_string_clear
#define string_dealloc char_string_dealloc
#define string_clone char_string_clone
#define string_reserve char_string_reserve
#define string_to_cstr char_string_to_cstr
#define string_push_utf8 char_string_push_utf8
#define string_concat char_string_concat

#endif // CHAR_STRIP_PREFIX

#endif // CHAR_STRIP_PREFIX_GUARD_
