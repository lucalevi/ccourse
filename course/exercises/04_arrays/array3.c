/*
 * array3: arrays as function arguments
 *
 * An array is not copied when you pass it to a function. The function
 * only receives the place in memory where the array starts. From that it
 * can reach the elements, but it has no way to tell how many there are.
 *
 * So the sizeof trick from array2 does not work inside the function: it
 * no longer measures the array. The rule in C is simple and universal:
 * whenever you pass an array, pass its length as well.
 *
 * TASK
 * Give sum a second parameter for the number of elements, and pass it
 * from main (where sizeof still works). The program must compile without
 * warnings and print:
 *
 *     sum = 100
 */

#include <stdio.h>

int sum(int values[]) {
    int count = sizeof(values) / sizeof(values[0]);
    int total = 0;

    for (int i = 0; i < count; i++) {
        total = total + values[i];
    }
    return total;
}

int main(void) {
    int values[] = {10, 20, 30, 40};

    printf("sum = %d\n", sum(values));
    return 0;
}
