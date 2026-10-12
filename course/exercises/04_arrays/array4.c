/*
 * array4: a function can change the caller's array
 *
 * You saw in value1 that a function gets a copy of an int argument and
 * cannot change the caller's variable.
 *
 * Arrays behave differently, for the reason given in array3: the function
 * is not handed a copy of the elements, only where they are. When it
 * writes to v[i], it writes into the caller's array.
 *
 * TASK
 * Do not change the code. Write what it prints between the marker lines.
 */

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
*/
