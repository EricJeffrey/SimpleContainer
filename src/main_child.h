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

// install busybox to /bin
void install_busybox() {
    LOGGER_DEBUG_SIMP("INSTALL BUSYBOX");
    int ret = Fork();
    if (ret == 0)
        Execl("/busybox", "busybox", "--install", "/bin", NULL);
    Waitpid(ret, NULL, 0);
}

// execute busybox inside container
void start_shell() {
    LOGGER_DEBUG_SIMP("CHILD: START SHELL");
    LOGGER_DEBUG_FORMAT("CHILD: GETUID: %d", getuid());

    Execl("/bin/sh", "sh", NULL);
}

// create path and write content to it
void create_file(const char *path, int oflag, const char *content) {
    LOGGER_DEBUG_FORMAT("CREATE FILE, PATH: %s", path);
    int fd = Open(path, oflag);
    Write(fd, content, strlen(content));
    Close(fd);
    Chmod(path, S_IRWXU | S_IRWXG);
}

// prepare files in /etc/
void prep_etc_files() {
    LOGGER_DEBUG_SIMP("PREPARE FILES IN /etc");

    create_file("/etc/group", O_CREAT | O_RDWR, "root:x:0:root\n");
    create_file("/etc/passwd", O_CREAT | O_RDWR, "root:x:0:0:root:/root:/bin/sh\n");
    create_file("/etc/resolv.conf", O_CREAT | O_RDWR, "nameserver 202.38.64.56\n");
}

void config_net_child() {
    LOGGER_DEBUG_SIMP("CHILD: CONFIG NETWORK");

    const char scripts[] = "\
    ifconfig %s 192.168.9.100 up\
    \nip route add default via 192.168.9.1 dev %s";
    const char veth2_name[] = "vethzxcv";

    const int buf_size = 1024;
    char buf[buf_size] = {};

    getcwd(buf, buf_size);
    LOGGER_DEBUG_FORMAT("CHILD: GETCWD: %s", buf);

    memset(buf, 0, sizeof(buf));
    int n = snprintf(buf, buf_size, scripts, veth2_name, veth2_name);

    const char config_child_path[] = "./net_config_child.sh";
    int fd = Open(config_child_path, O_RDWR | O_CREAT);
    Write(fd, buf, n);
    Close(fd);

    int ret = Fork();
    if (ret == 0)
        Execl("/bin/sh", "sh", config_child_path, NULL);
    Waitpid(ret, NULL, 0);
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
    Mount("proc", "/proc", "proc", 0, NULL);
    Umount2("/old_root", MNT_DETACH);

    prep_etc_files();

    install_busybox();
    config_net_child();
    start_shell();
}

#endif // MAIN_CHILD
