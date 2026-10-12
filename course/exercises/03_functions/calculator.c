/*
 * calculator: one function per job
 *
 * The prototypes and main are given. The four functions they promise are
 * not written yet. Until they are, the build fails at its last step, the
 * linker, which complains about "undefined symbols": things that were
 * declared and used but never defined.
 *
 * TASK
 * Write the four functions below main so that the program prints:
 *
 *     7 + 2 = 9.00
 *     7 - 2 = 5.00
 *     7 * 2 = 14.00
 *     7 / 2 = 3.50
 */

#include <stdio.h>

double add(double a, double b);
double subtract(double a, double b);
double multiply(double a, double b);
double divide(double a, double b);

int main(void) {
    printf("7 + 2 = %.2f\n", add(7, 2));
    printf("7 - 2 = %.2f\n", subtract(7, 2));
    printf("7 * 2 = %.2f\n", multiply(7, 2));
    printf("7 / 2 = %.2f\n", divide(7, 2));
    return 0;
}

/* TODO: define add, subtract, multiply and divide */
