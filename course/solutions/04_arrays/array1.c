/* An array of 5 elements has indexes 0 to 4. */

#include <stdio.h>

int main(void) {
    int primes[5] = {2, 3, 5, 7, 11};

    printf("first: %d\n", primes[0]);
    printf("last: %d\n", primes[4]);
    return 0;
}
