#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "errors.h"

void errmsg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    fflush(stderr);
    sleep(1);
    va_end(args);
}
