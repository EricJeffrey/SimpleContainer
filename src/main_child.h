/**
 * Jobs of Child in New Namespace
 * include install, mount, umount file system, etc.
 */

#if !defined(MAIN_CHILD)
#define MAIN_CHILD

#include "logger.h"
#include "ns_config.h"
#include "unistd.h"
#include "wait.h"
#include "wrappers.h"

// install busybox
void ins_busybox() {
    LOGGER_DEBUG_SIMP("CHILD: INSTALL BUSYBOX");

    int ret = Fork();
    if (ret == 0)
        Execl("/bin/busybox", "busybox", "--install", "/bin/", NULL);
    Waitpid(ret, NULL, 0);
}

// execute busybox inside container
void start_shell() {
    LOGGER_DEBUG_SIMP("CHILD: START SHELL");
    setuid(0);
    LOGGER_DEBUG_FORMAT("CHILD: GETUID: %d", getuid());

    Execl("/bin/sh", "sh", NULL);
}

// new namespace init
int new_ns_init(void *arg) {
    LOGGER_DEBUG_SIMP("CHILD: NAMESPACE INIT");

    // sync with map_user_grp
    int *pipe_fd = (int *)arg;
    Close(pipe_fd[1]);
    char ch;
    // block untile fd[1] is closed
    Read(pipe_fd[0], &ch, 1);
    LOGGER_DEBUG_SIMP("PIPE CLOSED");

    const char hostname[] = "busybox";
    Sethostname(hostname, strlen(hostname));

    Mount("./new_root", "./new_root", NULL, MS_BIND, NULL);
    Chdir("./new_root");

    Syscall(SYS_pivot_root, "./", "./old_root");

    Mount("none", "/proc", "proc", 0, NULL);
    // Umount("/old_root");

    ins_busybox();
    config_net_child();
    start_shell();
}

#endif // MAIN_CHILD
