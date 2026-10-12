/*
 * func1: declaring before calling
 *
 * A function is a named block of code you can run ("call") from elsewhere.
 * You have been writing one all along: main.
 *
 * The compiler reads a file from top to bottom, once. When it meets a
 * call, it must already know that the function exists and what it takes
 * and returns. There are two ways to arrange that:
 *
 *   - define the function above the place where it is called, or
 *   - put a prototype above the call: the function's first line followed
 *     by a semicolon instead of a body, for example
 *
 *         void greet(void);
 *
 *     The full definition can then come later in the file.
 *
 * void as the return type means "returns nothing"; void between the
 * parentheses means "takes no arguments".
 *
 * TASK
 * Make the program compile by adding a prototype. Leave main first.
 */

#include <stdio.h>

int main(void) {
    greet();
    greet();
    return 0;
}

void greet(void) {
    printf("Hello from a function!\n");
}
