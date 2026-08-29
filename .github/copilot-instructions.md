# Project Agent Operating Rules

Use the repository knowledge base before making design or implementation decisions.

## Project Context

这是 **YGOPRO Battleground** — 基于开源游戏王平台 ygopro 的**定制化服务端**二次开发项目。

- 上游项目: mycard/ygopro-core (决斗引擎), mycard/ygopro (客户端/服务器框架), Fluorohydride/ygopro-scripts (卡牌脚本)
- 开发重点: **服务器端定制化**，非客户端 UI 开发
- 语言栈: C++ (ocgcore + gframe), Lua (卡牌脚本), SQLite (卡牌数据)

## Required Reading Order

When a task is not trivial, inspect the most relevant files in this order:

1. `knowledge-base/project-overview.md`
2. The most relevant topic file under `knowledge-base/`
3. The nearest implementation files and tests in the repository

> 战旗玩法与平衡讨论参考 `knowledge-base/battleground-game-design.md`；技术架构参考 `knowledge-base/battleground-design.md`；里程碑、任务、依赖、状态和验收标准以 `knowledge-base/battleground-roadmap.md` 为唯一权威；接口、实现细节、风险和验证证据参考 `knowledge-base/battleground-tech.md`；编号规则与旧编号映射参考 `knowledge-base/task-numbering.md`。
> 文档编辑规范参考 `knowledge-base/doc-standards.md`。
> `memories/` 只用于快速恢复上下文，不得覆盖知识库中的任务状态、架构或玩法结论。

## Working Style

- Work in small, reviewable increments.
- Prefer the simplest design that satisfies the current task.
- Stay close to existing repository patterns unless the knowledge base says otherwise.
- Do not invent architecture, workflows, or tool choices when the repository already defines them.
- Before editing, identify one local hypothesis about where the behavior is controlled.
- After the first meaningful edit, run the narrowest validation that can fail for that change.
- Keep tests close to the behavior being changed.
- Update docs when the repository behavior or workflow meaningfully changes.

## Server-Side Customization Priorities

当用户要求服务端定制时，按以下优先级选择实现方案：

1. **纯配置** → `lflist.conf`（禁限卡表）、`strings.conf`（系统文本）、`config.h` 宏开关
2. **Lua 脚本** → `script/c{code}.lua`（卡牌效果，无需重编译 C++）
3. **gframe 服务器层** → `netserver.cpp`（网络行为）、`game.cpp`（`YGOPRO_SERVER_MODE` 分支）
4. **ocgcore 引擎** → `field.cpp`/`processor.cpp`（游戏规则变更，最高风险）

## Key Files For Server Development

| 改动场景      | 主要文件                                                               |
| ------------- | ---------------------------------------------------------------------- |
| 禁限卡表      | `lflist.conf`                                                          |
| 游戏提示/文本 | `strings.conf`                                                         |
| 卡牌效果      | `script/c{code}.lua`                                                   |
| 卡牌数据      | `cards.cdb` (SQLite)                                                   |
| 服务器网络    | `gframe/netserver.cpp`, `gframe/network.h`                             |
| 编译配置      | `gframe/config.h`, `premake5.lua`                                      |
| 核心引擎      | `ocgcore/field.cpp`, `ocgcore/processor.cpp`, `ocgcore/operations.cpp` |
| 脚本加载      | `ocgcore/interpreter.cpp`                                              |
| C-Lua 绑定    | `ocgcore/libcard.cpp`, `ocgcore/libduel.cpp`, `ocgcore/libeffect.cpp`  |
| 调试日志      | `debug.log` — 所有 stdout/stderr 重定向至此（`gframe.cpp` `freopen`）  |

## Debug Log 注意事项

- **服务器启动后 `fprintf(stderr, ...)` / `printf(...)` 输出不会出现在控制台**，全部写入 `debug.log`
- 查看日志：`Get-Content debug.log -Tail 50` 或直接打开 `debug.log`
- 搜索 `fprintf(stderr, ...)` 的输出请读取 `debug.log`，不要在终端/控制台找日志

## Safety And Scope

- Do not make unrelated changes.
- Do not rewrite large areas when a local fix is sufficient.
- If the knowledge base is missing critical information, ask for clarification or add a short assumption note in the chat.
- If the repository has project-specific commands, prefer those over generic commands.
- **服务器模式下不使用 Irrlicht 渲染**：`YGOPRO_SERVER_MODE` 宏下的代码路径不依赖 GUI
- **保持 ocgcore 上游兼容性**：尽量不修改决斗引擎核心逻辑

## Agent Collaboration

- Use role separation for planning, implementation, review, and verification when that improves clarity.
- Keep each agent narrow and opinionated.
- When generating new agents, prefer a small set of clearly distinct roles over many overlapping roles.
