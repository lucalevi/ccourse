/* Dividing by 3.0 instead of 3 makes it a floating-point division. */

#include <stdio.h>

int main(void) {
    int a = 3;
    int b = 4;
    int c = 4;

    double average = (a + b + c) / 3.0;

    printf("average = %.2f\n", average);
    return 0;
}
