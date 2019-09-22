/**
 * Configuration include uid/gid map or Network
 * 配置 uid_map gid_map 等文件
 */

#if !defined(NS_CONFIG)
#define NS_CONFIG

#include "logger.h"
#include "wrappers.h"

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

#endif // NS_CONFIG
