# 运行Alpine的简单容器

参考[busybox简单容器](https://segmentfault.com/a/1190000006913509)，使用C语言实现的轻量级容器，使用AlpineRootfs作为文件系统。

## 容器创建步骤
- 创建根目录，解压缩根文件系统
- 创建新的Namespace
- 映射用户/组id
- 配置网络 bridge + NAT
  - 挂载文件系统
  - 执行脚本配置网络
  - 启动shell
- 等待子进程终止

## TODO - 待办
1. Alpine根文件系统
2. 错误处理分级
      1. 严重错误 - 退出
      2. 不严重错误 - 日志

## 配置文件配置项
- 日志
  - 日志等级`LOG_LEVEL`
  - 日志输出文件，默认`stderr`
- 根文件系统路径
- 用户/组id映射值
- 网络设备名称
- 网络地址

## 已解决问题
1. `clone` 之后ssh连接失效/用户登出问题
   > 需要在终止时`kill`掉子进程，即`SIGCHLD`或`kill(0, SIGKILL)`
2. UID/GID映射写入失败
   > 需要由父进程向打开子进程的`/proc/PID/uid_map gid_map`文件，同时写入`gid_map`前需要向`/proc/PID/setgroups`写入字符串`"deny"`!
3. Namespace中不是root用户，显示unknown uid 0, 65534
   > 向`uid_map`, `gid_map`文件写入映射时需要使用`getuid`函数而不是简单的`0 1000 1`，即：`0 %ld 1, (long)getuid`, gid同样
4. 网络连接配置
   > 可以使用`ip route`命令添加路由表项，执行脚本完成Bridge + NAT配置即可。
5. Umount文件系统出现`resource busy`
   > 使用`umount -l target`或者`umount2(target, MNT_DETACH)`，**detach**而不卸载文件系统
6. 