/* is_prime starts as true and is switched off by the first divisor found. */

#include <stdio.h>

int main(void) {
    for (int n = 2; n < 30; n++) {
        int is_prime = 1;

        for (int d = 2; d < n; d++) {
            if (n % d == 0) {
                is_prime = 0;
                break;
            }
        }
        if (is_prime) {
            printf("%d\n", n);
        }
    }
    return 0;
}
