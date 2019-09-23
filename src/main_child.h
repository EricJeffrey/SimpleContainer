/**
 * Jobs of Child in New Namespace
 * include install, mount, umount file system, etc.
 */

#if !defined(MAIN_CHILD)
#define MAIN_CHILD

#include "logger.h"
#include "unistd.h"
#include "wait.h"
#include "wrappers.h"
#include <sys/capability.h>

/* ==================Alpine Root Fs====================*/

// read child's pid and wait pipe's close
pid_t childPipeReadWait(int *pipe_fd) {
    LOGGER_DEBUG_SIMP("CHILD: WAIT FOR PIPE");

    Close(pipe_fd[1]);

    const int pid_size = sizeof(pid_t);
    pid_t my_pid;
    if (Read(pipe_fd[0], &my_pid, pid_size) != pid_size)
        errExitSimp("CHILD: PIPE READ ERROR, READ PID");

    char ch;
    if (Read(pipe_fd[0], &ch, 1) != 0)
        errExitSimp("CHILD: PIPE READ ERROR, READ EOF");

    LOGGER_DEBUG_SIMP("CHILD: PIPE CLOSED");
    return my_pid;
}

void childPivotAndMount() {
    LOGGER_DEBUG_SIMP("CHILD: MOUNT FILESYSTEM");

    Mount("./rootfs/", "./rootfs/", NULL, MS_BIND, NULL);
    MD("./rootfs/old_root/");
    // [new_root] and [put_old] must not be on the same filesystem as the current root.
    // [put_old] must be underneath [new_root], that is, adding a nonzero number of /.. to the string  pointed  to  by
    // [put_old] must yield the same directory as [new_root].
    Chdir("./rootfs");
    Syscall(SYS_pivot_root, "./", "./old_root");
    Mount("none", "/dev", "tmpfs", 0, NULL);
    Mount("proc", "/proc", "proc", 0, NULL);
    Mount("sysfs", "/sys", "sysfs", 0, NULL);
    Mount("tmpfs", "/run", "tmpfs", 0, NULL);
    Umount2("/old_root", MNT_DETACH);
}

void childConfNet() {
    LOGGER_DEBUG_SIMP("CHILD: CONFIG NETWORK");

    const char veth2_name[] = "vethzxcv";
    const char config_child_path[] = "./net_config_child.sh";
    const char scripts[] = "\
    ifconfig %s 192.168.9.100 up\n\
    ip route add default via 192.168.9.1 dev %s";

    const int buf_size = 1024;
    char buf[buf_size] = {};
    int n = snprintf(buf, buf_size, scripts, veth2_name, veth2_name);

    int fd = Open(config_child_path, O_RDWR | O_CREAT);
    Write(fd, buf, n);
    Close(fd);

    int ret = Fork();
    if (ret == 0)
        Execl("/bin/sh", "sh", config_child_path, NULL);
    Waitp(ret);

    int dns_fd = Open("/etc/resolv.conf", O_RDWR | O_CREAT);
    char dns_conf_buf[] = "nameserver 8.8.8.8";
    Write(dns_fd, dns_conf_buf, strlen(dns_conf_buf));
}

void childStart() {
    LOGGER_DEBUG_SIMP("CHILD: START SHELL");

    LOGGER_DEBUG_FORMAT("CHILD: WHOAMI BEFORE SHELL: %ld", (long)getuid());

    Execl("/bin/sh", "sh", NULL);
}

void childSetEnv() {
    const char hostname[] = "alpine_fs";
    Sethostname(hostname, strlen(hostname));
}

int childInit(void *arg) {
    LOGGER_DEBUG_SIMP("CHILD: INIT");

    pid_t my_pid = childPipeReadWait((int *)arg);
    childPivotAndMount();
    childConfNet();
    childSetEnv();
    childStart();
}

/* ==================Alpine Root Fs====================*/

#endif // MAIN_CHILD
