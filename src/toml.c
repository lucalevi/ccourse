#include "toml.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *at;
    int line;
} Parser;

static bool error(const Parser *p, const char *what) {
    fail("line %d: %s", p->line, what);
    return false;
}

static void skip_spaces(Parser *p) {
    while (*p->at == ' ' || *p->at == '\t') {
        p->at++;
    }
}

static void skip_comment(Parser *p) {
    if (*p->at == '#') {
        p->at += strcspn(p->at, "\n");
    }
}

/* Skips spaces, comments and line breaks. */
static void skip_blank(Parser *p) {
    for (;;) {
        skip_spaces(p);
        skip_comment(p);
        if (*p->at == '\n') {
            p->line++;
        } else if (*p->at != '\r') {
            return;
        }
        p->at++;
    }
}

static bool is_name_char(char c) {
    return isalnum((unsigned char)c) || c == '_' || c == '-';
}

/* Reads a key or table name. The caller frees it. */
static char *parse_name(Parser *p, bool dotted) {
    const char *start = p->at;
    while (is_name_char(*p->at) || (dotted && *p->at == '.')) {
        p->at++;
    }
    if (p->at == start) {
        error(p, "expected a name");
        return NULL;
    }
    Str name = {0};
    str_addn(&name, start, (size_t)(p->at - start));
    return str_take(&name);
}

static void add_utf8(Str *s, unsigned long code) {
    char bytes[4];
    size_t len;
    if (code < 0x80) {
        bytes[0] = (char)code;
        len = 1;
    } else if (code < 0x800) {
        bytes[0] = (char)(0xC0 | (code >> 6));
        bytes[1] = (char)(0x80 | (code & 0x3F));
        len = 2;
    } else if (code < 0x10000) {
        bytes[0] = (char)(0xE0 | (code >> 12));
        bytes[1] = (char)(0x80 | ((code >> 6) & 0x3F));
        bytes[2] = (char)(0x80 | (code & 0x3F));
        len = 3;
    } else {
        bytes[0] = (char)(0xF0 | (code >> 18));
        bytes[1] = (char)(0x80 | ((code >> 12) & 0x3F));
        bytes[2] = (char)(0x80 | ((code >> 6) & 0x3F));
        bytes[3] = (char)(0x80 | (code & 0x3F));
        len = 4;
    }
    str_addn(s, bytes, len);
}

/* Reads what follows a backslash. */
static bool parse_escape(Parser *p, Str *s) {
    char c = *p->at;
    if (c != '\0') {
        p->at++;
    }
    switch (c) {
    case 'n': str_add(s, "\n"); return true;
    case 't': str_add(s, "\t"); return true;
    case 'r': str_add(s, "\r"); return true;
    case 'b': str_add(s, "\b"); return true;
    case 'f': str_add(s, "\f"); return true;
    case '"': str_add(s, "\""); return true;
    case '\\': str_add(s, "\\"); return true;
    case 'u':
    case 'U': {
        int digits = c == 'u' ? 4 : 8;
        unsigned long code = 0;
        for (int i = 0; i < digits; i++) {
            if (!isxdigit((unsigned char)*p->at)) {
                return error(p, "\\u needs 4 hexadecimal digits and \\U needs 8");
            }
            char digit[2] = {*p->at++, '\0'};
            code = code * 16 + strtoul(digit, NULL, 16);
        }
        if (code == 0 || code > 0x10FFFF) {
            return error(p, "\\u or \\U names a character that cannot be used");
        }
        add_utf8(s, code);
        return true;
    }
    default:
        return error(p, "unknown escape in string (the known ones are \\n \\t \\r \\\" \\\\ \\u \\U)");
    }
}

/* Reads a "basic" or 'literal' string. The caller frees it. */
static bool parse_string(Parser *p, char **out) {
    char quote = *p->at++;
    if (p->at[0] == quote && p->at[1] == quote) {
        return error(p, "multi-line strings are not supported");
    }
    Str s = {0};
    for (;;) {
        char c = *p->at;
        if (c == '\0' || c == '\n') {
            str_free(&s);
            return error(p, "string is missing its closing quote");
        }
        p->at++;
        if (c == quote) {
            break;
        }
        if (c == '\\' && quote == '"') {
            if (!parse_escape(p, &s)) {
                str_free(&s);
                return false;
            }
        } else {
            str_addn(&s, &c, 1);
        }
    }
    *out = str_take(&s);
    return true;
}

static bool is_quote(char c) {
    return c == '"' || c == '\'';
}

/* Reads a list of strings, which may span lines and end with a comma. */
static bool parse_strings(Parser *p, Strings *out) {
    p->at++;
    for (;;) {
        skip_blank(p);
        if (*p->at == ']') {
            break;
        }
        if (!is_quote(*p->at)) {
            return error(p, "expected a string or ] (lists may only hold strings)");
        }
        char *item;
        if (!parse_string(p, &item)) {
            return false;
        }
        strings_push(out, item);
        free(item);
        skip_blank(p);
        if (*p->at == ',') {
            p->at++;
        } else if (*p->at != ']') {
            return error(p, "expected , or ] in list");
        }
    }
    p->at++;
    return true;
}

static bool parse_word(Parser *p, const char *word) {
    size_t len = strlen(word);
    if (strncmp(p->at, word, len) != 0 || is_name_char(p->at[len])) {
        return false;
    }
    p->at += len;
    return true;
}

static bool parse_value(Parser *p, TomlValue *value) {
    char c = *p->at;
    if (is_quote(c)) {
        value->type = TOML_STRING;
        return parse_string(p, &value->string);
    }
    if (c == '[') {
        value->type = TOML_STRINGS;
        return parse_strings(p, &value->strings);
    }
    if (parse_word(p, "true") || parse_word(p, "false")) {
        value->type = TOML_BOOL;
        value->boolean = c == 't';
        return true;
    }
    if (isdigit((unsigned char)c) || c == '-' || c == '+') {
        char *end;
        errno = 0;
        value->type = TOML_INT;
        value->integer = strtol(p->at, &end, 10);
        if (end == p->at || errno != 0 || is_name_char(*end) || *end == '.') {
            return error(p, "expected a whole number");
        }
        p->at = end;
        return true;
    }
    return error(p, "expected a value: a string, a whole number, true, false or a list of strings");
}

static void value_free(TomlValue *value) {
    free(value->string);
    strings_free(&value->strings);
}

/* Reads `[[name]]` and reports it. */
static bool parse_table(Parser *p, const TomlHandler *handler, void *ctx) {
    if (p->at[1] != '[') {
        return error(p, "only [[name]] tables are supported, not [name]");
    }
    p->at += 2;
    skip_spaces(p);
    char *name = parse_name(p, true);
    if (name == NULL) {
        return false;
    }
    skip_spaces(p);
    bool ok = true;
    if (strncmp(p->at, "]]", 2) != 0) {
        ok = error(p, "expected ]] after the table name");
    } else if (handler->table == NULL) {
        ok = error(p, "tables are not expected in this file");
    } else if (!handler->table(ctx, name)) {
        ok = error(p, failure());
    }
    p->at += ok ? 2 : 0;
    free(name);
    return ok;
}

/* Reads `key = value` and reports it. */
static bool parse_pair(Parser *p, const TomlHandler *handler, void *ctx) {
    Parser start = *p;
    char *key = parse_name(p, false);
    if (key == NULL) {
        return false;
    }
    skip_spaces(p);
    TomlValue value = {0};
    bool ok = *p->at == '=' || error(p, "expected = after the key");
    if (ok) {
        p->at++;
        skip_spaces(p);
        ok = parse_value(p, &value);
    }
    if (ok && !handler->value(ctx, key, &value)) {
        ok = error(&start, failure());
    }
    value_free(&value);
    free(key);
    return ok;
}

bool toml_parse(const char *text, const TomlHandler *handler, void *ctx) {
    Parser p = {text, 1};
    for (;;) {
        skip_blank(&p);
        if (*p.at == '\0') {
            return true;
        }
        if (!(*p.at == '[' ? parse_table(&p, handler, ctx) : parse_pair(&p, handler, ctx))) {
            return false;
        }
        skip_spaces(&p);
        skip_comment(&p);
        if (*p.at != '\0' && *p.at != '\n' && *p.at != '\r') {
            return error(&p, "unexpected text at the end of the line");
        }
    }
}

void toml_add_string(Str *out, const char *text) {
    str_add(out, "\"");
    for (; *text != '\0'; text++) {
        switch (*text) {
        case '"': str_add(out, "\\\""); break;
        case '\\': str_add(out, "\\\\"); break;
        case '\n': str_add(out, "\\n"); break;
        case '\r': str_add(out, "\\r"); break;
        case '\t': str_add(out, "\\t"); break;
        default: str_addn(out, text, 1);
        }
    }
    str_add(out, "\"");
}
