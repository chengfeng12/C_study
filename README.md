# C_study

用「一个主题一个目录、一篇一个 `.c`」的方式系统过一遍 C 语言，可编译、可运行、注释即讲义。

## 目录

```
阶段1/   语法基础（入门与输入输出 → 变量 → 运算符 → 流程控制 → 数组 → 指针）
阶段2/   函数与程序结构（函数 → 结构体 → 共用体 → typedef → 日期与时间 → 动态内存）
```

具体到课次的结构见 [CONTRIBUTING.md](CONTRIBUTING.md#1-目录结构)。

## 编译运行

```bash
mkdir -p bin
gcc -std=c99 -Wall -Wextra 阶段1/01_入门与输入输出/01_hello.c -o bin/01_hello && ./bin/01_hello
```

## 仓库文档

| 文档 | 内容 |
|---|---|
| [CONTRIBUTING.md](CONTRIBUTING.md) | 目录结构、文件命名、文首注释格式、编译要求、不入库清单 |
| [GIT_COMMIT.md](GIT_COMMIT.md) | Conventional Commits 提交格式、type 取值、提交粒度、分支与合并 |
