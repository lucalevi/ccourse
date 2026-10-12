/*
 * types1: what does arithmetic really produce?
 *
 * Things to keep in mind:
 *   - int / int throws the fraction away; % gives the remainder.
 *   - * and / are done before + and -, as in school.
 *   - A char is stored as a small number (its character code), and the
 *     letters A, B, C... have consecutive codes.
 *   - sizeof gives the size of a type in bytes. Its result has the type
 *     size_t, which is printed with %zu. A char is always exactly 1 byte.
 *
 * TASK
 * Do not change the code. Write what it prints between the marker lines
 * at the bottom.
 */

#include <stdio.h>

int main(void) {
    printf("%d\n", 7 / 2);
    printf("%d\n", 7 % 2);
    printf("%.1f\n", 7 / 2.0);
    printf("%d\n", 2 + 3 * 4);
    printf("%c\n", 'A' + 2);
    printf("%zu\n", sizeof(char));
    return 0;
}

/* PREDICT
*/
