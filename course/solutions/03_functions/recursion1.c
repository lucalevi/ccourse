/* For n of 0 (or less) the answer is known, so the chain of calls ends. */

#include <stdio.h>

int factorial(int n) {
    if (n <= 0) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main(void) {
    printf("%d\n", factorial(0));
    printf("%d\n", factorial(5));
    printf("%d\n", factorial(10));
    return 0;
}
