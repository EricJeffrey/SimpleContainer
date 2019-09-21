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

// execute busybox inside container
void start_shell() {
    LOGGER_DEBUG_SIMP("CHILD: START SHELL");
    LOGGER_DEBUG_FORMAT("CHILD: GETUID: %d", getuid());

    Execl("/bin/sh", "sh", NULL);
}

// create /etc/passwd file and write root info to it
void create_pw_file() {
    LOGGER_DEBUG_SIMP("CREATE PASSWD FILE");

    const char pw_path[] = "/etc/passwd";
    const char pw_content[] = "root:x:0:0:root:/root:/bin/sh\neric:x:1000:1000:eric:/home/eric:/bin/sh\n";
    int fd = Open(pw_path, O_CREAT | O_RDWR);
    Write(fd, pw_content, strlen(pw_content));
    Close(fd);
}

// create /etc/passwd file and write root info to it
void create_grp_file() {
    LOGGER_DEBUG_SIMP("CREATE GROUP FILE");

    const char grp_path[] = "/etc/group";
    const char grp_content[] = "root:x:0:\neric:x:1000\n";
    int fd = Open(grp_path, O_CREAT | O_RDWR);
    Write(fd, grp_content, strlen(grp_content));
    Close(fd);
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

    create_pw_file();
    create_grp_file();
    Setuid((uid_t)0);

    // Umount("/old_root");

    // config_net_child();
    start_shell();
}

#endif // MAIN_CHILD
