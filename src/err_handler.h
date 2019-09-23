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

#define errExitSimp(A)                                                                \
    do {                                                                              \
        fprintf(stderr, "%s.\nerrno: %d, strerror: %s\n", A, errno, strerror(errno)); \
        kill(0, SIGKILL);                                                             \
        exit(EXIT_FAILURE);                                                           \
    } while (0);

#define errExitFormat(A, ...)                                                    \
    do {                                                                         \
        fprintf(stderr, A, __VA_ARGS__);                                         \
        fprintf(stderr, ".\nerrno: %d, strerror: %s\n", errno, strerror(errno)); \
        kill(0, SIGKILL);                                                        \
        exit(EXIT_FAILURE);                                                      \
    } while (0)

#endif // ERRH
