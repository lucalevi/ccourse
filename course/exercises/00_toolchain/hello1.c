/*
 * hello1: your first C program
 *
 * A C program starts running at the function called main. The lines between
 * its braces { } are statements, and every statement ends with a semicolon.
 *
 * printf prints text. Inside the text, \n means "start a new line".
 *
 * TASK
 * This program does not compile. Look at the compiler's message in the
 * ccourse window: it names the file, the line, the column, and what it
 * expected to find there. Fix the program so that it prints:
 *
 *     Hello, C!
 *
 * Save the file and ccourse checks it again by itself.
 */

#include <stdio.h>

int main(void) {
    printf("Hello, C!\n")
    return 0;
}
