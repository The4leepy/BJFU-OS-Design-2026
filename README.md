#### *项目正在推进中，目前完成了进程管理模块、内存管理模块。


# 项目使用说明 (Usage Guide)

欢迎使用本项目！本项目基于 C++20 编写，并依赖 `pthread` 库。

为了方便快速上手，项目已配置好完整的 `Makefile`。**在大多数配置好环境的系统中，您只需要在项目根目录下执行一行命令即可一键编译并运行**：

```bash
make run
```

请根据您所使用的操作系统，参考下方的具体环境配置与运行指南。

## 目录

- [支持的编译目标 (Make Targets)](#支持的编译目标-make-targets)
- [Linux 用户指南 (推荐)](#linux-用户指南-推荐)
- [macOS 用户指南](#macos-用户指南)
- [Windows 用户指南](#windows-用户指南)
- [常见问题与排查](#常见问题与排查)

---

## 支持的编译目标 (Make Targets)

在终端中，您可以按需使用以下命令：

| 命令                     | 作用                                                                             |
| :----------------------- | :------------------------------------------------------------------------------- |
| **`make run`**   | **【推荐】一键编译整个项目、清空终端屏幕并直接运行生成的 `os_sim` 程序** |
| `make` 或 `make all` | 仅编译项目，生成可执行文件 `os_sim`，不运行                                    |
| `make clean`           | 清理编译过程中生成的中间文件（`.o`）和最终程序                                 |

---

## Linux 用户指南 (推荐)

Linux 环境原生支持本项目所需的所有工具链。

### 1. 安装依赖

如果您的系统尚未安装 `g++` 和 `make`，请打开终端运行以下命令：

* **Ubuntu / Debian 系:**
  ```bash
  sudo apt update
  sudo apt install build-essential
  ```

### 2. 一键运行

在项目根目录下执行：

```bash
make run
```

---

## macOS 用户指南

macOS 自带完善的类 Unix 开发环境，首次使用需激活命令行工具。

### 1. 安装依赖

打开终端（Terminal），运行以下命令以安装 C++ 编译环境和 `make` 工具：

```bash
xcode-select --install
```

### 2. 一键运行

在项目根目录下执行：

```bash
make run
```

---

## Windows 用户指南

由于本项目的 `Makefile` 使用了 Linux/Unix 原生的 `rm -f` 清理命令和 `./` 运行路径，且可能包含系统级 API 调用，在 Windows 下**强烈建议使用 WSL 或 Git Bash** 来运行。

### 方案 A：使用 WSL (Windows 的 Linux 子系统) - 【最推荐】

1. 打开安装好的 WSL 终端（如 Ubuntu）。
2. 进入本项目所在的目录（例如 `cd /mnt/c/Users/你的用户名/Projects/项目名`）。
3. 如果未安装编译工具，执行 `sudo apt install build-essential`。
4. 执行一键运行命令：
   ```bash
   make run
   ```

### 方案 B：使用 Git Bash + MinGW

如果您不想使用 WSL，可以在安装了 Git for Windows 和 MinGW-w64 的前提下：

1. 在项目文件夹中右键选择 **"Open Git Bash here"**。
2. 确保您的 MinGW 环境变量已配置，且 `g++` 可用。
3. 输入 `mingw32-make run`（或如果您重命名过则直接输入 `make run`）。

> **⚠️ 注意**：原生 CMD 或 PowerShell 会因为无法识别 `Makefile` 中的 `rm -f` 和 `clear` 命令而导致 `make clean` 或 `make run` 报错，请务必使用上述两种类 Unix 环境。

---

## 常见问题与排查

### 1. 提示 "unrecognized command line option '-std=c++20'"

* **原因**：您的 `g++` 编译器版本过低，不支持 C++20 标准。
* **解决办法**：请升级您的 GCC/g++ 编译器版本（建议 GCC 10 及以上）。

### 2. 提示 "Makefile:xxx: *** 遗漏分隔符 。 停止。" (Missing separator)

* **原因**：Makefile 严格要求命令前必须使用 **Tab 键** 缩进，而不能使用空格。如果在下载过程中 Tab 被转换成了空格，就会报错。
* **解决办法**：在编辑器中打开 `Makefile`，将报错行前面的空格删掉，重新按一下 `Tab` 键保存即可。
