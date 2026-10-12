#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void *xrealloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size ? size : 1);
    if (ptr == NULL) {
        fputs("ccourse: out of memory\n", stderr);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *xmalloc(size_t size) {
    return xrealloc(NULL, size);
}

char *xstrdup(const char *text) {
    size_t size = strlen(text) + 1;
    return memcpy(xmalloc(size), text, size);
}

/* Makes room for `extra` more bytes and the terminating zero. */
static void str_reserve(Str *s, size_t extra) {
    size_t needed = s->len + extra + 1;
    if (needed <= s->cap) {
        return;
    }
    size_t cap = s->cap ? s->cap : 64;
    while (cap < needed) {
        cap *= 2;
    }
    s->data = xrealloc(s->data, cap);
    s->cap = cap;
}

void str_addn(Str *s, const char *text, size_t len) {
    str_reserve(s, len);
    memcpy(s->data + s->len, text, len);
    s->len += len;
    s->data[s->len] = '\0';
}

void str_add(Str *s, const char *text) {
    str_addn(s, text, strlen(text));
}

static void str_addv(Str *s, const char *fmt, va_list args) {
    va_list again;
    va_copy(again, args);
    int len = vsnprintf(NULL, 0, fmt, again);
    va_end(again);
    if (len < 0) {
        return;
    }
    str_reserve(s, (size_t)len);
    vsnprintf(s->data + s->len, (size_t)len + 1, fmt, args);
    s->len += (size_t)len;
}

void str_addf(Str *s, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    str_addv(s, fmt, args);
    va_end(args);
}

char *str_take(Str *s) {
    char *text = s->data ? s->data : xstrdup("");
    *s = (Str){0};
    return text;
}

void str_free(Str *s) {
    free(s->data);
    *s = (Str){0};
}

char *format(const char *fmt, ...) {
    Str s = {0};
    va_list args;
    va_start(args, fmt);
    str_addv(&s, fmt, args);
    va_end(args);
    return str_take(&s);
}

void strings_push(Strings *list, const char *text) {
    list->items = xrealloc(list->items, (list->count + 1) * sizeof *list->items);
    list->items[list->count++] = xstrdup(text);
}

bool strings_contains(const Strings *list, const char *text) {
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->items[i], text) == 0) {
            return true;
        }
    }
    return false;
}

void strings_free(Strings *list) {
    for (size_t i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    *list = (Strings){0};
}

Strings split_lines(const char *text) {
    Strings lines = {0};
    while (*text != '\0') {
        size_t len = strcspn(text, "\n");
        const char *next = text + len + (text[len] == '\n');
        if (len > 0 && text[len - 1] == '\r') {
            len--;
        }
        Str line = {0};
        str_addn(&line, text, len);
        lines.items = xrealloc(lines.items, (lines.count + 1) * sizeof *lines.items);
        lines.items[lines.count++] = str_take(&line);
        text = next;
    }
    return lines;
}

bool is_blank(const char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }
    return *text == '\0';
}

static char message[2048];

void fail(const char *fmt, ...) {
    /* Built aside first, so that the old message may be part of the new one. */
    char next[sizeof message];
    va_list args;
    va_start(args, fmt);
    vsnprintf(next, sizeof next, fmt, args);
    va_end(args);
    memcpy(message, next, sizeof message);
}

const char *failure(void) {
    return message;
}

bool is_file(const char *path) {
    struct stat info;
    return stat(path, &info) == 0 && S_ISREG(info.st_mode);
}

char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fail("cannot read %s: %s", path, strerror(errno));
        return NULL;
    }
    Str text = {0};
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, file)) > 0) {
        str_addn(&text, buf, n);
    }
    bool ok = !ferror(file);
    fclose(file);
    if (!ok) {
        fail("cannot read %s", path);
        str_free(&text);
        return NULL;
    }
    return str_take(&text);
}

bool write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        fail("cannot write %s: %s", path, strerror(errno));
        return false;
    }
    bool ok = fputs(text, file) != EOF;
    ok = fclose(file) == 0 && ok;
    if (!ok) {
        fail("cannot write %s", path);
    }
    return ok;
}

bool copy_file(const char *from, const char *to) {
    char *text = read_file(from);
    if (text == NULL) {
        return false;
    }
    bool ok = write_file(to, text);
    free(text);
    return ok;
}

bool make_dirs(const char *path) {
    if (*path == '\0') {
        return true;
    }
    char *copy = xstrdup(path);
    bool ok = true;
    /* Cut the path short at each slash in turn, then use it whole. */
    for (char *slash = copy + 1; ok; slash++) {
        slash += strcspn(slash, "/");
        char end = *slash;
        *slash = '\0';
        if (mkdir(copy, 0755) != 0 && errno != EEXIST) {
            fail("cannot create directory %s: %s", copy, strerror(errno));
            ok = false;
        }
        *slash = end;
        if (end == '\0') {
            break;
        }
    }
    free(copy);
    return ok;
}

bool make_parent_dirs(const char *path) {
    const char *slash = strrchr(path, '/');
    if (slash == NULL || slash == path) {
        return true;
    }
    Str parent = {0};
    str_addn(&parent, path, (size_t)(slash - path));
    bool ok = make_dirs(parent.data);
    str_free(&parent);
    return ok;
}

void sleep_ms(int ms) {
    struct timespec pause = {ms / 1000, (ms % 1000) * 1000000L};
    nanosleep(&pause, NULL);
}
