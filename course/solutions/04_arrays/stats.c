/* Start from the first element, not from 0, so negative data works too. */

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

int smallest(int v[], int n) {
    int result = v[0];

    for (int i = 1; i < n; i++) {
        if (v[i] < result) {
            result = v[i];
        }
    }
    return result;
}

int largest(int v[], int n) {
    int result = v[0];

    for (int i = 1; i < n; i++) {
        if (v[i] > result) {
            result = v[i];
        }
    }
    return result;
}

double mean(int v[], int n) {
    int sum = 0;

    for (int i = 0; i < n; i++) {
        sum = sum + v[i];
    }
    return (double) sum / n;
}
