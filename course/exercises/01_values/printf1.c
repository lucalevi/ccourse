/*
 * printf1: types and format specifiers
 *
 * The types you will use most:
 *
 *     int      whole numbers            print with %d
 *     double   numbers with a fraction  print with %f
 *     char     a single character       print with %c
 *
 * A char value is written in single quotes: 'L'.
 *
 * printf cannot see the types of the values you pass. It believes the
 * specifier. If they disagree, the output is nonsense.
 *
 * %f prints six decimals. To choose how many, write a precision:
 * %.2f prints two decimals, %.1f prints one.
 *
 * TASK
 * Fix the specifiers so the program prints:
 *
 *     age: 36
 *     height: 1.82 m
 *     initial: L
 */

#include <stdio.h>

int main(void) {
    int age = 36;
    double height = 1.82;
    char initial = 'L';

    printf("age: %f\n", age);
    printf("height: %d m\n", height);
    printf("initial: %d\n", initial);
    return 0;
}
