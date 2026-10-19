---
description: "Use when writing or modifying code, tests, or build scripts. Covers small-step delivery, validation, testability, avoiding speculative design, and this repository's build commands, language rules, and high-risk areas."
applyTo: "**/*.{c,cc,cpp,h,hpp,lua,conf,cdb,ps1,txt,vcxproj,filters}"
---

# Engineering Guidance

## 通用纪律

- Work in small, reviewable increments.
- Prefer the simplest design that supports the current behavior.
- Keep testable logic separate from environment-heavy boundaries when practical.
- Run the narrowest relevant verification before considering the task complete.
- Prefer project-local commands and caches when the repository defines them.
- Do not hand-edit generated artifacts unless the repository explicitly expects that workflow.

## YGOPRO Battleground 专属约束

> 本节是各 agent 共用的唯一副本。agent 定义只描述职责与边界，不再重复项目约束。

### 实现层级优先级

配置 > Lua 脚本 > gframe 服务器层 > ocgcore 引擎。能用低风险层解决时不得下沉到引擎层。

### C++ 规范（ocgcore + gframe）

- 标准：C++98/C++11，保持与上游兼容
- 缩进：**Tab**，不是空格
- 命名：`snake_case` 文件名/函数/变量，`PascalCase` 类名
- 包含顺序：本地头文件 → 系统头文件
- 内存：栈分配 + RAII 优先；`new`/`delete` 必须配对
- 错误处理：条件检查 + 提前返回，**禁用 C++ 异常**
- 注释：中文（`RAII`、`handler` 等英文技术术语可保留原文）
- 不可修改 `premake/` 下的第三方库

### Lua 脚本规范

- 文件 `c{8位密码}.lua`，入口 `c{code}.initial_effect(c)`
- 回调命名 `c{code}.funcname`（用 `.` 而非 `:`）
- 使用 `aux.*` 辅助（`utility.lua`），常量取自 `constant.lua`
- 禁止 `require` / `dofile`（沙箱限制）
- 效果三要素：condition / target / operation
- 注释：中文

### 配置与清理格式

- `strings.conf`：`!system <id> <文本>`，`%ls` 占位符数量必须与运行时参数匹配
- `lflist.conf`：`!<版本>` + `<code> <count> --<名称>`
- 注释行统一用 `#`，不混用其他注释风格
- 不改 `cards.cdb` schema

### 构建与验证

```powershell
# premake5.exe 在仓库根目录；PowerShell 必须用 .\ 前缀；不要 cd ocgcore
# ocgcore 改动
.\premake5 vs2022; msbuild build/ocgcore.vcxproj /p:Configuration=Release

# gframe/server 改动（需 YGOPRO_SERVER_MODE）
.\premake5 vs2022; msbuild build/ygopro.vcxproj /p:Configuration=Release

# 启动验证（stdout/stderr 写入 debug.log，控制台看不到）
./ygopro server
```

验证顺序见 `knowledge-base/testing.md`：编译 → 启动 → 功能 → 回归。

### 服务器模式特殊说明

- `YGOPRO_SERVER_MODE` 宏下不依赖 Irrlicht 渲染
- 网络层基于 libevent 异步 I/O（`gframe/network.h`）
- `PRO_VERSION` 在 `gframe/config.h`，不可随意变更

### 高风险区域

1. `ocgcore/field.cpp`、`processor.cpp`、`operations.cpp` — 规则核心，可能破坏全部卡牌行为
2. `ocgcore/interpreter.cpp` — Lua 加载与沙箱（`dofile`/`loadfile` 逃逸）
3. `gframe/netserver.cpp` — 并发安全与缓冲区边界（`SIZE_NETWORK_BUFFER = 0x20000`）
4. `ocgcore/libcard.cpp`、`libduel.cpp`、`libeffect.cpp` — C-Lua 绑定，参数校验影响全部脚本
5. `cards.cdb` — 数据一致性
