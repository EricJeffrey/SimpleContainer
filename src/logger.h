/**
 * Logger Util
 * Name：ALL CAP + UNDERLINE
 * By EricJeffrey
*/

#if !defined(LOGGER)
#define LOGGER
#include <stdio.h>
#include <stdlib.h>

#define LOG_LV_DEBUG 1
#define LOG_LV_VERBOSE 2

int LOG_LEVEL = 0;

FILE *FP_LOG_OUTPUT = stderr;

// 输出字符串到日志文件
#define LOGGER_VERB_SIMP(A)                       \
    do {                                          \
        if (LOG_LEVEL >= LOG_LV_VERBOSE)          \
            fprintf(FP_LOG_OUTPUT, "%s...\n", A); \
    } while (0)

// 输出格式化字符串到日志文件
#define LOGGER_VERB_FORMAT(fmt, ...)                  \
    do {                                              \
        if (LOG_LEVEL >= LOG_LV_VERBOSE) {            \
            fprintf(FP_LOG_OUTPUT, fmt, __VA_ARGS__); \
            fprintf(FP_LOG_OUTPUT, "\n");             \
        }                                             \
    } while (0)

#define LOGGER_DEBUG_SIMP(A)                      \
    do {                                          \
        if (LOG_LEVEL >= LOG_LV_DEBUG)            \
            fprintf(FP_LOG_OUTPUT, "%s...\n", A); \
    } while (0)


#define LOGGER_DEBUG_FORMAT(A, ...)                 \
    do {                                            \
        if (LOG_LEVEL >= LOG_LV_DEBUG) {            \
            fprintf(FP_LOG_OUTPUT, A, __VA_ARGS__); \
            fprintf(FP_LOG_OUTPUT, "\n");           \
        }                                           \
    } while (0)

// 设置日志等级
#define LOGGER_SET_LV(x)                              \
    do {                                              \
        if (x >= LOG_LV_DEBUG && x <= LOG_LV_VERBOSE) \
            LOG_LEVEL = x;                            \
    } while (0)

const void logger_debug_formt() {}

#endif // LOGGER
