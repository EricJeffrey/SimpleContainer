/**
 * Wrappers, handler error
 * Naming: Big Camel
 */
#if !defined(WRAPPERS)
#define WRAPPERS

#include "err_handler.h"
#include "logger.h"
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>
#include <wait.h>

void Mkdir(const char *path, mode_t mode) {
    LOGGER_VERB_SIMP("Mkdir");
    if (mkdir(path, mode) < 0)
        errExitFormat("MKDIR ERROR, PATH: %s, MODE: %d", path, mode);
}

void Chmod(const char *path, mode_t mode) {
    LOGGER_VERB_SIMP("Chmod");
    if (chmod(path, mode) < 0)
        errExitFormat("CHMOD ERROR, PATH: %s", path);
}

void Sethostname(const char *name, size_t len) {
    LOGGER_VERB_SIMP("Sethostname");
    if (sethostname(name, len) < 0)
        errExitFormat("SETHOSTNAME ERROR, NAME: %s", name);
}

void Setuid(uid_t uid) {
    LOGGER_VERB_SIMP("SETUID");

    if (setuid(uid) < 0)
        errExitFormat("SETUID ERROR, UID: %ld", (long)uid);
}

void Chdir(const char *path) {
    LOGGER_VERB_SIMP("Chdir");
    if (chdir(path) < 0)
        errExitFormat("CHDIR ERROR, PATH: %s", path);
}

pid_t Fork() {
    LOGGER_VERB_SIMP("Fork");
    int ret = fork();
    if (ret < 0)
        errExitSimp("FORK ERROR");
    return ret;
}

pid_t Waitpid(pid_t pid, int *stat_loc, int options) {
    LOGGER_VERB_SIMP("Waitpid");
    int ret = waitpid(pid, stat_loc, options);
    if (ret < 0)
        errExitFormat("WAITPID ERROR, PID: %d", pid);
    return ret;
}

pid_t Waitp(pid_t pid) {
    LOGGER_VERB_SIMP("Waitpid With NULL stat_loc And 0 options");
    int ret = waitpid(pid, NULL, 0);
    if (ret < 0)
        errExitFormat("WAITPID ERROR, PID: %d", pid);
    return ret;
}

int Open(const char *path, int oflag) {
    LOGGER_VERB_SIMP("Open");
    int ret = open(path, oflag);
    if (ret < 0)
        errExitFormat("OPEN ERROR, PATH: %s, OFLAG: %d", path, oflag);
    return ret;
}

void Close(int fd) {
    LOGGER_VERB_SIMP("Close");
    if (close(fd) < 0)
        errExitFormat("CLOSE ERROR, FD: %d", fd);
}

int Read(int fd, void *buf, size_t nbytes) {
    LOGGER_VERB_SIMP("Read");
    int ret = 0;
    if ((ret = read(fd, buf, nbytes)) < 0)
        errExitFormat("READ ERROR, FD: %d", fd);
    return ret;
}

int Write(int fd, const void *buf, size_t n) {
    LOGGER_VERB_SIMP("Write");
    int ret = write(fd, buf, n);
    if (ret < 0)
        errExitFormat("WRITE ERROR, FD: %d", fd);
    return ret;
}

void Fstat(int fd, struct stat *buf) {
    LOGGER_VERB_SIMP("FSTAT");

    if (fstat(fd, buf) < 0)
        errExitFormat("FSTAT ERROR, FD: %d", fd);
}

void Copy_file_range(int infd, off64_t *pinoff, int outfd, off64_t *poutoff, size_t len) {
    LOGGER_VERB_SIMP("COPY FILE RANGE");

    if (copy_file_range(infd, pinoff, outfd, poutoff, len, 0) < 0)
        errExitFormat("COPY FILE RANGE, INFD: %d, OUTFD: %d", infd, outfd);
}

void Mount(const char *spec_file, const char *dir, const char *fstype, unsigned long rwflag, const void *data) {
    LOGGER_VERB_FORMAT("Mount, SPEC_FILE: %s, DIR: %s, fstype: %s", spec_file, dir, fstype);

    if (mount(spec_file, dir, fstype, rwflag, data) < 0)
        errExitFormat("MOUNT ERROR, SPEC_FILE: %s, DIR: %s, FSTYPE: %s", spec_file, dir, fstype);
}

void Umount(const char *spec_file) {
    LOGGER_VERB_FORMAT("Umount, SPEC_FILE: %s", spec_file);

    if (umount(spec_file) < 0)
        errExitFormat("UMOUNT ERROR, SPEC_FILE: %s", spec_file);
}

void Umount2(const char *spec_file, int flags) {
    LOGGER_VERB_FORMAT("UMOUNT2, SEPC_FILE: %s", spec_file);

    if (umount2(spec_file, flags) < 0)
        errExitFormat("UMOUNT2 ERROR, SEPC_FILE: %s", spec_file);
}

int Clone(int (*fn)(void *), void *child_stack, int flags, void *arg) {
    LOGGER_VERB_SIMP("Clone");

    int ret = clone(fn, child_stack, flags, arg);
    if (ret < 0)
        errExitFormat("CLONE ERROR, FLAGS: %d", flags);
    return ret;
}

void Pipe(int *pipefds) {
    LOGGER_VERB_SIMP("Pipe");
    if (pipe(pipefds) < 0)
        errExitSimp("PIPE CREATE ERROR");
}
#define Syscall(sysno, ...)                    \
    do {                                       \
        LOGGER_VERB_SIMP("SYSCALL");           \
        if (syscall(sysno, __VA_ARGS__) < 0) { \
            errExitSimp("SYSCALL ERROR");      \
        }                                      \
    } while (0)

#define Execl(path, arg, ...)                                       \
    do {                                                            \
        LOGGER_VERB_SIMP("EXECL");                                  \
        execl(path, arg, __VA_ARGS__);                              \
        errExitFormat("EXECL ERROR, PATH: %s, ARG: %s", path, arg); \
    } while (0)

#define MD(A) Mkdir(A, S_IRWXU | S_IRWXG)

#endif // WRAPPERS
