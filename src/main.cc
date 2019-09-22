#include "main_child.h"

const int STACK_SIZE = 1024 * 1024;
char child_stack[STACK_SIZE];

#define MD(A) Mkdir(A, S_IRWXU | S_IRWXG)

// 将 old_path 指向的文件复制到 new_path，出错时返回 -1
void CopyFile(const char old_path[], const char new_path[]) {
    int oldfd = Open(old_path, O_RDONLY);
    int newfd = Open(new_path, O_RDWR | O_CREAT);
    struct stat stbuf;
    Fstat(oldfd, &stbuf);
    Copy_file_range(oldfd, NULL, newfd, NULL, stbuf.st_size);
    Close(oldfd);
    Close(newfd);

    Chmod(new_path, S_IRWXU | S_IRWXG);
}
// make needed files and dirs, this will move cwd to simp_contianer_root
// and install busybox inside /bin
void prepare_files() {
    LOGGER_DEBUG_SIMP("PREPARE DIRS");

    MD("../simp_container_root/");
    Chdir("../simp_container_root/");
    MD("./new_root");
    MD("./new_root/bin");
    MD("./new_root/proc");
    MD("./new_root/old_root");
    MD("./new_root/etc");
    MD("./new_root/root");
    MD("./new_root/home");
    CopyFile("/home/eric/coding/busybox", "./new_root/busybox");
}

// make root dir and move program to root/bin
void create_container() {
    LOGGER_DEBUG_SIMP("CREATE CONTAINER");

    prepare_files();

    int pipe_fd[2];
    Pipe(pipe_fd);

    int ns_flag = 0;
    pid_t child_pid;

    LOGGER_DEBUG_SIMP("CLONE CHILD");
    ns_flag |= CLONE_NEWCGROUP;
    ns_flag |= CLONE_NEWIPC;
    ns_flag |= CLONE_NEWNET;
    ns_flag |= CLONE_NEWNS;
    ns_flag |= CLONE_NEWPID;
    ns_flag |= CLONE_NEWUTS;
    ns_flag |= CLONE_NEWUSER;
    ns_flag |= SIGCHLD;
    child_pid = Clone(new_ns_init, child_stack + STACK_SIZE, ns_flag, pipe_fd);

    map_ugid(child_pid);
    config_net_p(child_pid);
    Close(pipe_fd[1]);

    LOGGER_DEBUG_FORMAT("CLONE OVER, PARENT -- MY_PID: %ld, CHILD_PID: %ld\n", long(getpid()), long(child_pid));
    Waitpid(child_pid, NULL, 0);

    LOGGER_DEBUG_SIMP("I APPEAR RIGHT AFTER CHILD DIE, BEFORE PARENT EXITING");
    kill(0, SIGKILL);
    exit(0);
}

int main(int argc, char const *argv[]) {
    LOGGER_SET_LV(LOG_LV_DEBUG);
    create_container();
    return 0;
}
