/**
 * Error Handler
*/

#if !defined(ERRH)
#define ERRH

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const void errExit(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, fmt, ap);
    fprintf(stderr, ".\nerrno: %d, strerror: %s\n", errno, strerror(errno));
    va_end(ap);
    kill(0, SIGKILL);
    exit(EXIT_FAILURE);
}

#endif // ERRH
