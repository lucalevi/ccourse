/* A reader for the part of TOML that the course files use:
 *
 *   # comments
 *   [[array.of.tables]]
 *   key = "string"  |  'string'  |  123  |  true  |  ["list", "of", "strings"]
 *
 * It reports what it finds, in file order, to two callbacks.
 */

#ifndef CCOURSE_TOML_H
#define CCOURSE_TOML_H

#include <stdbool.h>

#include "util.h"

typedef enum { TOML_STRING, TOML_INT, TOML_BOOL, TOML_STRINGS } TomlType;

/* Only the member that matches `type` is meaningful. */
typedef struct {
    TomlType type;
    char *string;
    long integer;
    bool boolean;
    Strings strings;
} TomlValue;

/* Both callbacks return false, after calling fail(), to stop with an error.
 * The value belongs to the parser; copy what you want to keep. */
typedef struct {
    /* A `[[name]]` line. May be NULL if no tables are expected. */
    bool (*table)(void *ctx, const char *name);
    bool (*value)(void *ctx, const char *key, const TomlValue *value);
} TomlHandler;

/* On error, failure() starts with the line number. */
bool toml_parse(const char *text, const TomlHandler *handler, void *ctx);

/* The text as a quoted TOML string, added to `out`. */
void toml_add_string(Str *out, const char *text);

#endif
