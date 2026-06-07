# OS Core Simulator — 可持久化操作系统核心模拟器

北京林业大学 2025-2026 操作系统 A 课程设计

基于 C++20 实现的可持久化操作系统核心模拟器，支持进程管理、多级反馈队列调度、动态分区内存分配、二进制状态持久化、多线程架构、多实例并发访问及完整可视化。

---

## 快速开始

```bash
make run
```

首次启动需设置 root 密码。之后可通过 `login` / `register` 创建用户。

---

## 全部命令

### 用户管理

| 命令 | 说明 |
|------|------|
| `register <user> <pwd>` | 注册新用户（需 root） |
| `login <user>` | 登录 |
| `logout` | 登出 |
| `sudo <command>` | 以 root 权限执行单条命令 |
| `sudo su` | 切换到 root |
| `unlock <user>` | 解锁被锁定的用户（需 root） |
| `show_users` | 列出所有用户及状态 |

### 进程管理

| 命令 | 说明 |
|------|------|
| `create <name> <priority>` | 创建进程（优先级 0-15，0 最高） |
| `show <pid>` | 查看进程详情 |
| `list` | 列出当前用户的所有进程 |
| `renice <pid> <prio>` | 修改进程优先级 |
| `block <pid>` | 阻塞进程 |
| `wakeup <pid>` | 唤醒阻塞进程 |
| `suspend <pid>` | 挂起进程 |
| `resume <pid>` | 恢复挂起进程 |
| `ptree` | 树形显示进程父子关系 |
| `kill <pid>` | 终止进程及其子进程 |

### 调度器

| 命令 | 说明 |
|------|------|
| `step` | 单步执行一个时钟周期 |
| `start` | 启动后台自动调度 |
| `stop` | 停止后台自动调度 |
| `restart` | 重启后台调度 |

### 内存管理

| 命令 | 说明 |
|------|------|
| `alloc <pid> <size>` | 为进程分配内存（KB） |
| `free_mem <pid>` | 释放进程的全部内存 |
| `show_mem` | 显示内存分区表及可视化映射 |
| `compact` | 执行内存紧缩 |
| `mem_stat` | 显示内存统计及碎片率 |
| `set_alloc_algo <ff\|bf\|wf>` | 切换分配算法 |
| `pgfault` | 模拟缺页中断 |
| `swap_out <pid> <size>` | 将进程内存标记为换出 |

### 系统

| 命令 | 说明 |
|------|------|
| `overview` | 系统全景快照（进程树 + 内存图 + MLFQ） |
| `help [category]` | 查看帮助（可选 process/memory/scheduler/user） |
| `clear` | 清屏 |
| `exit` | 退出 |

---

## 核心特性

### 进程管理

- 固定 3 个系统进程：swapper(0) / init(1) / kthreadd(2)
- 用户创建的进程为 init 的子进程
- pid < 3 的系统进程不可被 kill/block/suspend 等
- swapper(0) 仅在无进程可运行时被调度

### MLFQ 调度器

- 3 级队列：Q0(prio 0-3,切片1) → Q1(prio 4-7,切片2) → Q2(prio 8-15,切片4)
- 时间片用尽自动降级
- `renice` 可改变优先级并移动队列
- `start` 后后台自动调度（~1s/tick），`step` 单步

### 内存管理

- 1024KB 动态分区，基于双向链表
- 首次适应 / 最佳适应 / 最坏适应 三种算法
- 释放自动合并相邻空闲分区
- `compact` 紧缩碎片

### 持久化

- 二进制格式保存/恢复全部状态
- 每条命令后自动保存（master 实例）
- 启动时询问是否加载上次状态
- viewer 实例静默自动同步

### 多线程

- 前台线程：接收用户命令 → 放入共享消息队列 → 等待结果
- 后台线程：从队列取消息 → 加锁执行 → 返回结果
- `std::mutex` + `condition_variable` 保证线程安全

### 多实例

- `flock` 文件锁：先获得锁的为 master，其余为 viewer
- master 负责维护状态、执行命令、自动保存
- viewer 只能观察，自动重载状态文件

---

## 编译目标

| 命令 | 作用 |
|------|------|
| `make run` | 一键编译并运行 |
| `make` | 仅编译，生成 `os_sim` |
| `make clean` | 清理编译产物 |

---

## 环境配置

### Linux

```bash
sudo apt install build-essential
make run
```

### macOS

```bash
xcode-select --install
make run
```

### Windows

推荐 WSL：
```bash
sudo apt install build-essential
make run
```
或使用 Git Bash + MinGW-w64：`mingw32-make run`

---

## 常见问题

| 问题 | 解决 |
|------|------|
| `-std=c++20` 不支持 | 升级 GCC 10+ 或 Clang 12+ |
| Makefile 分隔符错误 | Makefile 命令前必须用 Tab 缩进 |
| 程序启动报错 | 确保 `pthread` 库可用 |
