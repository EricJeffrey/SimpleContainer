# 超简易容器

参考[busybox简单容器](https://segmentfault.com/a/1190000006913509)，使用C语言实现的轻量级容器。

**新的Namespace中需要文件系统从而确定root用户、用户权限等，仅仅安装busybox会使许多命令无法使用**

## 容器创建步骤
1. 创建根目录、bin目录等
   1. 创建`/`, `/bin`目录
2. 配置根文件系统目录/root
   1. `/root`, `/old_root`
3. 挂载文件系统procfs等
   1. `/proc`, `mount`, `pivot_root`, `chroot`
4. 配置用户/组ID映射
5. 配置网络连接bridge + NAT
6. 从配置文件创建容器

## TODO - 待办
1. Namespace中不是root用户，无用户配置信息
2. 重构
   1. 添加包装函数处理错误 √
   2. 添加调试、verbose等信息 √
   3. 错误处理分级
      1. 严重错误 - 退出
      2. 不严重错误 - 日志
3. Alpine根文件系统
4. 配置网络连接
5. 添加`LOG/DEBUG`选项机制
6. 添加包装函数

## 已解决问题
1. ~~`clone` 之后ssh连接失效/用户登出问题~~ : 需要在终止时`kill`掉子进程，即`SIGCHLD`或`kill(0, SIGKILL)`
2. ~~UID/GID映射写入失败~~: 需要由父进程向打开子进程的`/proc/PID/uid_map gid_map`文件，同时写入`gid_map`前需要向`/proc/PID/setgroups`写入字符串`"deny"`!

## 配置文件配置项
- 日志
  - 日志等级`LOG_LEVEL`
  - 日志输出文件，默认`stderr`