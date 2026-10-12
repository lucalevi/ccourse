/*
 * stats: functions over arrays
 *
 * TASK
 * Write the three functions promised by the prototypes. Each receives an
 * array and its length.
 *
 *   - smallest returns the lowest value,
 *   - largest returns the highest value,
 *   - mean returns the average, as a double.
 *
 * The program must print:
 *
 *     min = -3
 *     max = 42
 *     mean = 12.50
 *
 * Take care not to read past the end of the array: the sanitizer is
 * watching. And remember what happens when an int is divided by an int.
 */

#include <stdio.h>

int smallest(int v[], int n);
int largest(int v[], int n);
double mean(int v[], int n);

int main(void) {
    int data[] = {7, -3, 42, 10, 19, 0};
    int n = sizeof(data) / sizeof(data[0]);

    printf("min = %d\n", smallest(data, n));
    printf("max = %d\n", largest(data, n));
    printf("mean = %.2f\n", mean(data, n));
    return 0;
}

/* TODO: define smallest, largest and mean */
