/* %d for int, %.2f for a double with two decimals, %c for char. */

#include <stdio.h>

int main(void) {
    int age = 36;
    double height = 1.82;
    char initial = 'L';

    printf("age: %d\n", age);
    printf("height: %.2f m\n", height);
    printf("initial: %c\n", initial);
    return 0;
}
