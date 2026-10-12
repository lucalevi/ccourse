/* Small helpers shared by the whole tool: strings, errors and files. */

#ifndef CCOURSE_UTIL_H
#define CCOURSE_UTIL_H

#include <stdbool.h>
#include <stddef.h>

#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"
#define DIM "\x1b[2m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define CYAN "\x1b[36m"

#if defined(__GNUC__)
#define PRINTF(fmt, args) __attribute__((format(printf, fmt, args)))
#else
#define PRINTF(fmt, args)
#endif

/* These end the program when memory runs out, so they never return NULL. */
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *text);

/* A string that grows as text is added. Start with `Str s = {0};`. */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} Str;

void str_add(Str *s, const char *text);
void str_addn(Str *s, const char *text, size_t len);
void str_addf(Str *s, const char *fmt, ...) PRINTF(2, 3);
/* Hands the text over to the caller, who must free it. Never NULL. */
char *str_take(Str *s);
void str_free(Str *s);

/* A new string built like printf. The caller frees it. */
char *format(const char *fmt, ...) PRINTF(1, 2);

/* A list of strings. Start with `Strings list = {0};`. */
typedef struct {
    char **items;
    size_t count;
} Strings;

void strings_push(Strings *list, const char *text);
bool strings_contains(const Strings *list, const char *text);
void strings_free(Strings *list);

/* Text as lines, without the line endings. A final line break adds no line. */
Strings split_lines(const char *text);

/* True if the text is empty or only whitespace. */
bool is_blank(const char *text);

/* Functions that return false or NULL on error first record why with fail(). */
void fail(const char *fmt, ...) PRINTF(1, 2);
const char *failure(void);

bool is_file(const char *path);
/* The whole file as a string the caller frees, or NULL. */
char *read_file(const char *path);
bool write_file(const char *path, const char *text);
bool copy_file(const char *from, const char *to);
/* Creates a directory and any missing parents. */
bool make_dirs(const char *path);
/* Creates the directory that `path` is in. */
bool make_parent_dirs(const char *path);

void sleep_ms(int ms);

#endif
