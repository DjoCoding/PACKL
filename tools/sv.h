#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "error.h"
#include "dyn-arr.h"

#define SV(s) sv_from_cstr(s)
#define SV_GET(b, s) (String_View) { .content = b, .count = s }
#define SV_FROM_CHAR(c) sv_from_char(c)
#define SV_NULL   (String_View) { .content = NULL, .count = 0 }
    
#define SV_FMT "%.*s"
#define SV_UNWRAP(s) (int)s.count, s.content

typedef struct String_View String_View;

struct String_View {
    char *content;
    size_t count;
};

typedef struct String_Slices String_Slices;

struct String_Slices {
    String_View *items;
    size_t count;
    size_t size;
};

String_View sv_from_cstr(char *s);
char *cstr_from_sv(String_View s);
String_View sv_from_file(char *filename);
String_View sv_trim_left(String_View s);
String_View sv_trim_right(String_View s);
String_View sv_trim(String_View s);
int sv_eq(String_View s, String_View t);
String_View sv_from_char(char c);
bool sv_empty(String_View s);
char sv_at(String_View s, size_t index);
int64_t sv_find_index(String_View s, char c);
size_t sv_count_char(String_View s, char c);
bool sv_starts_with(String_View s, char c);
bool sv_ends_with(String_View s, String_View t);
bool sv_is_integer(String_View s);
bool sv_is_float(String_View s);
bool sv_parse_integer(String_View s, int64_t *result);
bool sv_parse_float(String_View s, double *result);
String_Slices sv_split_by_delim(String_View s, int (*compare)(int));
String_Slices sv_get_lines(String_View s);
String_Slices sv_split(String_View s);
String_View sv_chop_left(String_View s);
String_View sv_chop_right(String_View s);
String_View sv_chop_both(String_View s);
char *unescape_string(char *s, size_t s_size);
String_View unescape_string_to_sv(String_View s);
char *sv_escape(String_View s);


#endif // STRING_VIEW_H