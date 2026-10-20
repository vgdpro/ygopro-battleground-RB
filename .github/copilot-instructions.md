# Project Agent Operating Rules

Use the repository knowledge base before making design or implementation decisions.

## Project Context

这是 **YGOPRO Battleground** — 基于开源游戏王平台 ygopro 的**定制化服务端**二次开发项目。

- 上游项目: mycard/ygopro-core (决斗引擎), mycard/ygopro (客户端/服务器框架), Fluorohydride/ygopro-scripts (卡牌脚本)
- 开发重点: **服务器端定制化**，非客户端 UI 开发
- 语言栈: C++ (ocgcore + gframe), Lua (卡牌脚本), SQLite (卡牌数据)

## Required Reading Order

When a task is not trivial, inspect the most relevant files in this order:

1. `knowledge-base/README.md`
2. The most relevant topic file under `knowledge-base/`
3. The nearest implementation files and tests in the repository

> 战旗玩法与平衡讨论参考 `knowledge-base/battleground-game-design.md`；技术架构与设计决策参考 `knowledge-base/battleground-design.md`；接口、实现行为与已知风险参考 `knowledge-base/battleground-tech.md`；已实现能力与 `M*` 编号参考 `knowledge-base/battleground-implemented.md`。
> 编码规范、构建命令与高风险区域见 `.github/instructions/engineering.instructions.md`；文档格式与编号规则见 `.github/instructions/doc-maintainer.instructions.md`。
> `memories/` 只用于快速恢复上下文，不得覆盖知识库中的架构或玩法结论。

## Output Style

**默认生效，无需触发。** 对用户的每条回复按 `.agents/skills/ste-zh/SKILL.md` 的规则写：

- 关键信息先行，一句一事，短句
- 状态词固定：已完成 / 部分完成 / 未开始 / 进行中 / 已验证 / 未验证 / 失败 / 跳过 / 阻塞 / 未确认
- 如实写验证状态；未执行的检查写「未验证」
- 不叙述过程，证据写 `路径:行号` 或完整命令
- 待决定事项列编号选项，推荐项第一

只管**对话回复**。代码、注释、commit message、写入项目的文件不受约束。

用户说「停止 ste」或「stop ste」后才恢复默认写法。

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
