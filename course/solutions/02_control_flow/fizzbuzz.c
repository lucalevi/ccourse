/* The combined case is tested first, or it would never be reached. */

#include <stdio.h>

int main(void) {
    for (int n = 1; n <= 15; n++) {
        if (n % 3 == 0 && n % 5 == 0) {
            printf("FizzBuzz\n");
        } else if (n % 3 == 0) {
            printf("Fizz\n");
        } else if (n % 5 == 0) {
            printf("Buzz\n");
        } else {
            printf("%d\n", n);
        }
    }
    return 0;
}
