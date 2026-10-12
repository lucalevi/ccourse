/* The array's elements were changed in place; the int was only a copy. */

#include <stdio.h>

void double_all(int v[], int n) {
    for (int i = 0; i < n; i++) {
        v[i] = v[i] * 2;
    }
}

void double_one(int x) {
    x = x * 2;
    printf("in function: %d\n", x);
}

int main(void) {
    int numbers[3] = {1, 2, 3};
    int single = 5;

    double_all(numbers, 3);
    double_one(single);

    printf("%d %d %d\n", numbers[0], numbers[1], numbers[2]);
    printf("%d\n", single);
    return 0;
}

/* PREDICT
in function: 10
2 4 6
5
*/
