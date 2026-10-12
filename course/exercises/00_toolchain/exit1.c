/*
 * exit1: the exit code
 *
 * The number that main returns is handed back to whoever started the
 * program. It is called the exit code. By convention 0 means "all went
 * well" and any other number means "something failed".
 *
 * In a shell you can see the exit code of the last program with:
 *
 *     echo $?
 *
 * Scripts and other programs rely on it, so a program that fails should
 * never return 0.
 *
 * TASK
 * This program reports a failure but then claims success. Make it exit
 * with code 3.
 */

#include <stdio.h>

int main(void) {
    printf("Could not open the file.\n");
    return 0;
}
