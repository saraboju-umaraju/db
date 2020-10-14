#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#define ch *( buf + var )

long long int
_atoi(char *buf)
{

    int var = 0;
    long long int data = 0;
    int minus_flag = 1;

    while (*buf == ' ' || *buf == '\t') {
        buf++;
    }

    if (*buf == '-') {
        minus_flag = -1;
        var++;
    }

    if (*buf == '+') {
        var++;
    }

    while (ch) {
        if (ch == ' ' || ch == '\t' || ch == '\n') {
            /* nothing to do */
        } else if (isdigit(ch)) {
            ch = ch - '0';
            data *= 10;
            data += (ch);
        }
        else {
            exit(1);
            break;
        }
        var++;
    }

    return data * minus_flag;
}
