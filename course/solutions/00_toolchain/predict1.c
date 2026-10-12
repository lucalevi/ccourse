/* printf only moves to a new line where the text contains \n. */

#include <stdio.h>

int main(void) {
    printf("one ");
    printf("two\n");
    printf("three\nfour\n");
    printf("fi");
    printf("ve\n");
    return 0;
}

/* PREDICT
one two
three
four
five
*/
