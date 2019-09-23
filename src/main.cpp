#include "main_child.h"

const int STACK_SIZE = 1024 * 1024;
char child_stack[STACK_SIZE];

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

/* ===================Alpine Rootfs===========================*/

// update uid/gid map file
void updateMap(const char *map_file_path, char const *content) {
    LOGGER_DEBUG_SIMP("UPDATE MAP");

    int fd_uid = Open(map_file_path, O_RDWR);
    int len = strlen(content);
    int ret = Write(fd_uid, content, len);
    Close(fd_uid);
}

// create rootfs and extract files
void prepareRootFs() {
    LOGGER_DEBUG_SIMP("PREPARE ROOT FS");

    const char def_container_path[] = "/home/eric/coding/simp_containers/";
    const char container_path[] = "/home/eric/coding/simp_containers/alpine_container/";
    const char rfs_path[] = "/home/eric/coding/simp_containers/alpine_container/rootfs/";
    const char rfs_file_path[] = "/home/eric/coding/alpine_rfs.tar.gz";

    MD(container_path);
    MD(rfs_path);
    Chdir(container_path);

    int ret = Fork();
    if (ret == 0)
        Execl("/bin/tar", "tar", "-xf", rfs_file_path, "-C", rfs_path, NULL);
    Waitp(ret);
}

// create new Namespace using clone()
pid_t createNewNs(int pipe_fd[]) {
    LOGGER_DEBUG_SIMP("CREATE NEW NAMESPACE");

    int ns_flags = 0;
    ns_flags |= CLONE_NEWCGROUP;
    ns_flags |= CLONE_NEWUTS;
    ns_flags |= CLONE_NEWIPC;
    ns_flags |= CLONE_NEWUSER;
    ns_flags |= CLONE_NEWPID;
    ns_flags |= CLONE_NEWNET;
    ns_flags |= CLONE_NEWNS;
    ns_flags |= SIGCHLD;
    pid_t child_pid = Clone(childInit, child_stack + STACK_SIZE, ns_flags, (void *)pipe_fd);
    return child_pid;
}

// write uid_map and gid_map
void mapUsrGrpId(pid_t child_pid) {
    LOGGER_DEBUG_SIMP("PARENT: MAP UID GID");
    const char uid_map_fmt[] = "0 %ld 100";
    const char gid_map_fmt[] = "0 %ld 100";

    const int buf_size = 1024;
    char uid_map_content[buf_size] = {};
    char gid_map_content[buf_size] = {};
    snprintf(uid_map_content, buf_size, uid_map_fmt, (long)getuid());
    snprintf(gid_map_content, buf_size, gid_map_fmt, (long)getgid());

    char uid_path[buf_size] = {};
    snprintf(uid_path, buf_size, "/proc/%ld/uid_map", (long)child_pid);
    updateMap(uid_path, uid_map_content);

    char setgrp_path[buf_size] = {};
    snprintf(setgrp_path, buf_size, "/proc/%ld/setgroups", (long)child_pid);
    updateMap(setgrp_path, "deny");

    char gid_path[buf_size] = {};
    snprintf(gid_path, buf_size, "/proc/%ld/gid_map", (long)child_pid);
    updateMap(gid_path, gid_map_content);
}

void confNetParent(pid_t child_pid) {
    LOGGER_DEBUG_SIMP("PARENT: CONFIG NETWORK");

    const char br_name[] = "simpbr0";
    const char veth1_name[] = "vethasdf", veth2_name[] = "vethzxcv";
    const char scripts_fmt[] = "\
    ip link add %s type bridge\n\
    ip link add %s type veth peer name %s\n\
    ip link set %s master %s\n\
    ip link set %s netns %ld\n\
    ifconfig %s 192.168.9.1/24 up\n\
    ifconfig %s up\n\
    iptables -t nat -A POSTROUTING -s 192.168.9.0/24 ! -d 192.168.9.0/24 -j MASQUERADE";

    const int buf_size = 1024;
    char buf[buf_size] = {};
    int n = snprintf(buf, buf_size, scripts_fmt, br_name, veth1_name, veth2_name,
                     veth1_name, br_name, veth2_name, (long)child_pid, br_name, veth1_name);

    const char config_p_path[] = "./net_config_p.sh";
    int fd = Open(config_p_path, O_RDWR | O_CREAT);
    Write(fd, buf, n);
    Close(fd);

    int ret = Fork();
    if (ret == 0)
        Execl("/bin/bash", "bash", config_p_path, NULL);
    Waitp(ret);
}

void createContainer() {
    LOGGER_DEBUG_SIMP("CREATE CONTAINER");

    int pipe_fd[2];
    Pipe(pipe_fd);

    prepareRootFs();
    int child_pid = createNewNs(pipe_fd);
    mapUsrGrpId(child_pid);
    confNetParent(child_pid);

    Close(pipe_fd[1]);
    Waitp(child_pid);
    kill(0, SIGKILL);
    exit(0);
}

/* ===================Alpine Rootfs===========================*/

int main(int argc, char const *argv[]) {
    LOGGER_SET_LV(LOG_LV_DEBUG);
    createContainer();
    return 0;
}
