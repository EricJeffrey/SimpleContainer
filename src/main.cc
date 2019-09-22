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

// update uid/gid map file
void update_map(const char *map_file_path, char const *content) {
    LOGGER_DEBUG_SIMP("UPDATE MAP");

    int fd_uid = Open(map_file_path, O_RDWR);
    int len = strlen(content);
    int ret = Write(fd_uid, content, len);
    Close(fd_uid);
}

// map uid and gid, 0 1000 10
void map_ugid(pid_t child_pid) {
    LOGGER_DEBUG_SIMP("MAP UID GID");

    const int buf_size = 30;
    char buf[buf_size] = {};
    snprintf(buf, buf_size, "0 %ld 1", (long)getuid());

    char id_path[buf_size] = {};

    snprintf(id_path, buf_size, "/proc/%ld/uid_map", (long)child_pid);
    update_map(id_path, buf);

    // write deny to /proc/$$/setgroups
    char setgrp_path[buf_size] = {};
    snprintf(setgrp_path, buf_size, "/proc/%ld/setgroups", (long)child_pid);
    update_map(setgrp_path, "deny");

    memset(id_path, 0, strlen(id_path));
    memset(buf, 0, sizeof(buf));
    snprintf(buf, buf_size, "0 %ld 1", (long)getgid());
    snprintf(id_path, buf_size, "/proc/%ld/gid_map", (long)child_pid);
    update_map(id_path, buf);
}

// config needed network within parent, bridge, veth1 veth2 and ip
void config_net_p(pid_t child_pid) {
    LOGGER_DEBUG_SIMP("CONFIG NETWORK in PARENT");

    const char br_name[] = "simpbr0";
    const char veth1_name[] = "vethasdf", veth2_name[] = "vethzxcv";
    const char scripts[] = "\
    ip link add %s type bridge\n\
    ip link add %s type veth peer name %s\n\
    ip link set %s master %s\n\
    ip link set %s netns %ld\n\
    ifconfig %s 192.168.9.1/24 up\n\
    ifconfig %s up\n\
    iptables -t nat -A POSTROUTING -s 192.168.9.0/24 ! -d 192.168.9.0/24 -j MASQUERADE";

    const int buf_size = 1024;
    char buf[buf_size] = {};
    int n = snprintf(buf, buf_size, scripts, br_name, veth1_name, veth2_name, veth1_name, br_name, veth2_name, (long)child_pid, br_name, veth1_name);

    const char config_p_path[] = "./net_config_p.sh";
    int fd = Open(config_p_path, O_RDWR | O_CREAT);
    Write(fd, buf, n);
    Close(fd);

    int ret = Fork();
    if (ret == 0)
        Execl("/bin/bash", "bash", config_p_path, NULL);
    Waitpid(ret, NULL, 0);
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
