/* The length is measured in main and travels with the array. */

#include <stdio.h>

int sum(int values[], int count) {
    int total = 0;

    for (int i = 0; i < count; i++) {
        total = total + values[i];
    }
    return total;
}

int main(void) {
    int values[] = {10, 20, 30, 40};
    int count = sizeof(values) / sizeof(values[0]);

    printf("sum = %d\n", sum(values, count));
    return 0;
}
