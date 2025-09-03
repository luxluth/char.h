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

#ifndef CharArg
#define CharArg(c) (int)(c).utf8_len, (c).data
#endif /* CharArg */

#ifndef char_S
#define char_S(cstr) char_string_from_cstr(cstr)
#endif /* char_S */

thread_local static uint8_t __char_temp[4];

// It is a dynamic array of utf-8 codepoints
// * count - represent byte size
// * len   - represent the string length
//
// This structure can be printed as follow:
//     printf(StrFmt"\n", StrArg(str));
typedef struct {
  bool __locked;
  uint8_t *items;
  size_t count;
  size_t capacity;

  size_t len;
} Char_String;

typedef struct {
  Char_String *str;
  size_t index;

  size_t __offset;
} Char_StringIter;

typedef struct {
  uint8_t data[4];
  size_t utf8_len;
} Char_Char;

// Create a string from the default c string
CHARDEF Char_String char_string_from_cstr(const char *chars);

// Create a string from raw parts
CHARDEF Char_String char_string_from_raw_data(const uint8_t *chars,
                                              size_t count);

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

// Create a code-point iterator starting at the beginning.
// Locking modifications to the string
CHARDEF Char_StringIter char_string_iter_begin(Char_String *s);

// Unlock modifications to the string, and clearing the iterator
CHARDEF void char_string_iter_end(Char_StringIter *it);

// Advance iterator by one code point.
// Writes decoded code point into `out_char`. out_char can be `NULL`
// Returns `false` at end
CHARDEF bool char_string_iter_next(Char_StringIter *it, Char_Char *out_char);

// Retrieve the index-th character (0-based).
// Return `false` when the data is out of bounds
// Complexity: O(n)
CHARDEF bool char_string_char_at(const Char_String *s, size_t index,
                                 Char_Char *out_char);

// Insert a code point before position index.
// - Shifts subsequent bytes forward.
// - Returns false if index out of range
CHARDEF bool char_string_insert(Char_String *s, const Char_String other,
                                size_t at_index);

// Remove character at position `index`.
// - Shifts subsequent bytes backward.
CHARDEF bool char_string_remove(Char_String *s, size_t index);

// Extract substring of len code points starting at `start`.
// - Stores result in `out`.
// - Returns false if range invalid.
CHARDEF bool char_string_substring(const Char_String *s, size_t start,
                                   size_t len, Char_String *out);

#endif // CHAR_H_

/* #define CHAR_IMPLEMENTATION */
// use #define CHAR_IMPLEMENTATION
// to include also the library implementation
#ifdef CHAR_IMPLEMENTATION

static inline size_t __size_t_safe_sub(size_t x) {
  if (x == 0)
    return 0;
  return x - 1;
}

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

CHARDEF bool __char_utf8_decode_next(const uint8_t *p, size_t width,
                                     uint8_t out[4], size_t *read) {
  if (__char_utf8_validate(p, width, read)) {
    size_t n = *read;
    // Copy bytes out
    if (out != NULL) {
      for (size_t i = 0; i < n; ++i)
        out[i] = p[i];
    }

    return true;
  } else {
    return false;
  }
}

// Convert code-point index -> byte offset.
CHARDEF bool __char_string_byte_offset_for_index(const Char_String *s,
                                                 size_t index, size_t *off,
                                                 size_t *prev_off) {

  size_t previous_offset = 0;
  size_t offset = 0;
  size_t current_index = 0;
  if (index == 0)
    goto terminate;
  for (;;) {
    size_t read = 0;
    if (__char_utf8_decode_next(s->items + offset, s->count - offset, NULL,
                                &read)) {
      previous_offset = offset;
      offset += read;
      if (current_index == index)
        break;
      current_index += 1;
    } else {
      return false;
    }
  }
terminate:
  if (prev_off != NULL) {
    *prev_off = previous_offset;
  }
  if (off != NULL)
    *off = offset;
  return true;
}

CHARDEF Char_String char_string_from_raw_data(const uint8_t *chars,
                                              size_t count) {
  Char_String str = {0};
  char_string_push_utf8(&str, chars, count);

  return str;
}

CHARDEF Char_String char_string_from_cstr(const char *chars) {
  size_t len = strlen(chars);
  return char_string_from_raw_data((const uint8_t *)chars, len);
}

CHARDEF void char_string_clear(Char_String *str) {
  if (!str->__locked) {
    str->count = 0;
    str->len = 0;
  }
}

CHARDEF void char_string_dealloc(Char_String *str) {
  if (!str->__locked) {
    char_da_free(str);
    str->len = 0;
    str->count = 0;
    str->items = NULL;
    str->capacity = 0;
  }
}

CHARDEF Char_String char_string_clone(const Char_String *s) {
  Char_String new_string = {0};
  char_da_append_many(&new_string, s->items, s->count);
  new_string.len = s->len;
  new_string.__locked = false;
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
  if (str->__locked)
    return false;
  size_t i = 0;
  while (i < len) {
    size_t read = 0;

    if (!__char_utf8_decode_next(p + i, len - i, __char_temp, &read)) {
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

CHARDEF Char_StringIter char_string_iter_begin(Char_String *s) {
  s->__locked = true;

  return (Char_StringIter){
      .str = s,
  };
}

CHARDEF void char_string_iter_end(Char_StringIter *it) {
  it->str->__locked = false;
  it->index = 0;
  it->__offset = 0;
  it->str = NULL;
}

CHARDEF bool char_string_iter_next(Char_StringIter *it, Char_Char *out_char) {
  Char_Char fallback_char = {0};
  Char_Char *used_char = &fallback_char;
  if (out_char != NULL) {
    used_char = out_char;
  }

  if (__char_utf8_decode_next(it->str->items + it->__offset,
                              it->str->count - it->__offset, used_char->data,
                              &used_char->utf8_len)) {

    it->__offset += used_char->utf8_len;
    it->index += 1;
    return true;
  } else {
    return false;
  }
}

CHARDEF bool char_string_char_at(const Char_String *s, size_t index,
                                 Char_Char *out_char) {
  if (index >= s->len)
    return false;

  Char_Char fallback_char = {0};
  Char_Char *used_char = &fallback_char;
  if (out_char != NULL) {
    used_char = out_char;
  }

  size_t offset = 0, current_index = 0;
  for (;;) {
    if (__char_utf8_decode_next(s->items + offset, s->count - offset,
                                used_char->data, &used_char->utf8_len)) {
      offset += used_char->utf8_len;
      if (current_index == index)
        break;
      current_index += 1;
    } else {
      return false;
    }
  }

  return true;
}

CHARDEF bool char_string_insert(Char_String *s, const Char_String other,
                                size_t at_index) {
  bool result = true;

  if (at_index > s->len || s->__locked) {
    return false;
  }

  Char_String prev_clone = char_string_clone(s);
  size_t offset_at = 0;

  if (!__char_string_byte_offset_for_index(s, __size_t_safe_sub(at_index),
                                           &offset_at, NULL)) {
    result = false;
    goto cleanup;
  }

  s->len = at_index;
  s->count = offset_at;

  char_da_append_many(s, other.items, other.count);
  s->len += other.len;

  char_da_append_many(s, prev_clone.items + offset_at,
                      prev_clone.count - offset_at);

  s->len += prev_clone.len - at_index;

cleanup:
  char_string_dealloc(&prev_clone);
  return result;
}

CHARDEF bool char_string_remove(Char_String *s, size_t index) {
  bool result = true;

  if (index >= s->len || s->__locked)
    return false;

  Char_String prev_clone = char_string_clone(s);
  size_t offset_at = 0; // end of target cp
  size_t prev_off = 0;  // start of target cp

  if (!__char_string_byte_offset_for_index(s, index, &offset_at, &prev_off)) {
    result = false;
    goto cleanup;
  }

  s->count = prev_off; // keep prefix

  char_da_append_many(s, prev_clone.items + offset_at, // append tail
                      prev_clone.count - offset_at);
  s->len = prev_clone.len - 1;

cleanup:
  char_string_dealloc(&prev_clone);
  return result;
}

CHARDEF bool char_string_substring(const Char_String *s, size_t start,
                                   size_t len, Char_String *out) {
  if (start > s->len || (start + len) > s->len) {
    return false;
  }

  size_t start_offset = 0, end_offset = 0;

  if (!__char_string_byte_offset_for_index(s, start, NULL, &start_offset))
    return false;
  if (!__char_string_byte_offset_for_index(s, start + len, NULL, &end_offset))
    return false;

  Char_String sub_str = {0};
  sub_str.len = len;
  char_da_append_many(&sub_str, s->items + start_offset,
                      end_offset - start_offset);

  *out = sub_str;
  return true;
}

#endif // CHAR_IMPLEMENTATION

#ifndef CHAR_STRIP_PREFIX_GUARD_
#define CHAR_STRIP_PREFIX_GUARD_

#ifdef CHAR_STRIP_PREFIX
#define S char_S
#define String Char_String
#define StringIter Char_StringIter
#define Char Char_Char
#define string_from_raw_data char_string_from_raw_data
#define string_from_cstr char_string_from_cstr
#define string_clear char_string_clear
#define string_dealloc char_string_dealloc
#define string_clone char_string_clone
#define string_reserve char_string_reserve
#define string_to_cstr char_string_to_cstr
#define string_push_utf8 char_string_push_utf8
#define string_concat char_string_concat
#define string_iter_begin char_string_iter_begin
#define string_iter_end char_string_iter_end
#define string_iter_next char_string_iter_next
#define string_char_at char_string_char_at
#define string_insert char_string_insert
#define string_remove char_string_remove
#define string_substring char_string_substring

#endif // CHAR_STRIP_PREFIX

#endif // CHAR_STRIP_PREFIX_GUARD_

/* ----------------------------------------------------------------------------
MIT - license

Copyright (c) 2025 Delphin Blehoussi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
Great Thanks to the Alexey Kutepov
-----------------------------------------------------------------------------*/
