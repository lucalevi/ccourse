/*
 * func3: return types are a promise
 *
 * A function whose return type is int must return an int on every path.
 * Computing a value is not enough: without the word return, the result
 * is thrown away and the caller receives garbage.
 *
 * A function whose return type is void must not return a value at all.
 * It may still use a bare "return;" to leave early.
 *
 * TASK
 * Both functions below break their promise. Fix them so the program
 * compiles without warnings and prints:
 *
 *     ----
 *     49
 *     ----
 */

#include <stdio.h>

int square(int n) {
    n * n;
}

void print_line(void) {
    printf("----\n");
    return 0;
}

int main(void) {
    print_line();
    printf("%d\n", square(7));
    print_line();
    return 0;
}
