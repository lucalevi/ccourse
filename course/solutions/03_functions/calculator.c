/* The parameters are double, so 7 / 2 keeps its fraction here. */

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

double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    return a / b;
}
