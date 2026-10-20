---
description: "Use when planning, implementing, reviewing, or generating agents for this repository. Requires consulting the local knowledge base before making repo-specific decisions."
applyTo: "**"
---

# Knowledge Base Usage

- Treat `knowledge-base/` as the repository-specific source of truth for product context, architecture, workflow, standards, and testing expectations.
- Before making non-trivial changes, read the most relevant knowledge-base files instead of guessing.
- If repository code conflicts with the knowledge base, call out the conflict and prefer the implementation only after checking whether the docs are stale.
- When generating agents or prompts, encode the repository's actual constraints from `knowledge-base/` rather than generic best practices.
- Keep generated agents small. Each one should have a distinct role, minimal tools, and explicit boundaries.

## 知识库文件索引

| 文件                          | 类型 | 内容                                       |
| ----------------------------- | ---- | ------------------------------------------ |
| `README.md`                   | 通用 | 项目定位、核心流程、文件索引               |
| `architecture.md`             | 通用 | 模块结构、边界、数据流、不变量             |
| `testing.md`                  | 通用 | 测试布局、验证顺序、陷阱、发布门禁         |
| `battleground-game-design.md` | 战旗 | **玩法设计**：规则语义、平衡目标、`BAL-*`  |
| `battleground-design.md`      | 战旗 | **架构设计**：单 duel 多 field、field 分配 |
| `battleground-tech.md`        | 战旗 | **技术实现**：接口、实现行为、已知风险     |
| `battleground-implemented.md` | 战旗 | **已实现清单**：`M*` 编号台账              |

## 战旗模式知识库使用规则

- `battleground-game-design.md` 是**玩法权威**：记录玩家可感知规则、设计理由和平衡目标；未经用户确认的提案不得写成既定规则
- `battleground-design.md` 是**设计权威**：任何架构决策必须先对照此文档
- `battleground-tech.md` 是**实现事实权威**：记录接口、实现行为、文件归属和风险，不维护状态与验证日志
- `battleground-implemented.md` 是**编号与已实现权威**：只记录已完成能力；未实现内容不登记，编号不连续即为未实现或已废弃
- 编号规则（`M*` / `BAL-*`）与文档格式规范统一维护在 `.github/instructions/doc-maintainer.instructions.md`
- Swarm 维护分工：Specifier 输出范围与改动说明；Orchestrator 输出已确认架构决策；Reviewer 输出架构审查与验证结论；仅 Doc Maintainer 编辑知识库，并在状态显著变化时同步 `memories/`
- `memories/` 是非权威恢复摘要；发生冲突时以对应 knowledge-base 权威文件为准
- 文档格式清理和过期内容删除由 **doc-maintainer** agent 负责（斜杠 `/doc-maintainer` 调用，或编辑知识库文件时自动加载 `.github/instructions/doc-maintainer.instructions.md`)

## 本地技能库（`.agents/skills`）

- **来源**：本地技能根目录 `D:\SourceCode\skills`，下设 4 个独立仓库，通过 Windows 目录联接（junction）挂载到 `.agents/skills/<skill-name>`，**不复制文件**
- **更新**：在任一源仓库执行 `git pull` 后，本项目链接内容自动生效；不维护第二份副本，不生成锁定文件
- **定位**：通用方法与检查清单来源。技能与仓库规范冲突时，**一律以本仓库知识库与 instructions 为准**
- **不适用**：技能库中无 Lua 专用技能、无 SQLite 专用技能、无 C++ 单元测试框架技能；`testing-patterns` 为 Jest 专用，不得用于 `ocgcore-tests`
- **边界**：目录联接不是安全边界，不得链接密钥、`.env` 或 `skills` 目录之外的任何内容

### 源仓库路由

| 源仓库              | 类型    | 技能根                                      | 定位                                      |
| ------------------- | ------- | ------------------------------------------- | ----------------------------------------- |
| `skills\skills`     | plugin  | `skills\engineering`、`skills\productivity` | **工程流程**，代码工作首选，短小可组合    |
| `agent-skills-hub`  | catalog | `skills\`                                   | **栈特定模式**（C/C++、架构、文档、玩法） |
| `simplify-codebase` | single  | 仓库根                                      | **代码简化 / 死代码清理**，独立学科       |
| `ste-zh`            | single  | 仓库根                                      | **Agent 输出风格**（强制，所有仓库）      |

- 同一主题两边都覆盖时**优先 plugin 仓库**（更短、为组合而写）
- 源仓库路由按**工作性质**而非技能数量决定
- 仓库根目录的 `hallmark` 是网页视觉设计权威，本项目为 C++ 服务端、无视觉界面，**不在范围内**

### 按角色分配

> 「源」列：`P` = `skills\skills`（plugin）、`C` = `agent-skills-hub`（catalog）、`S` = `simplify-codebase`、`Z` = `ste-zh`。
>
> **`ste-zh` 是强制技能**：所有仓库都必须链接，不由候选清单决定，不得因项目类型或任务大小省略。它是唯一的 Agent 输出风格技能；目录用途：汇报、进度、排查结论、待决定事项。与项目文档技能（`markdown-mermaid-writing`、`documentation-templates`）不同，后者管写入项目的文件，不适用于对话输出。

| 技能                             | 源  | 用途                                                               | 主要角色                                                |
| -------------------------------- | --- | ------------------------------------------------------------------ | ------------------------------------------------------- |
| `codebase-design`                | P   | 深模块设计、接口与接缝                                             | Coder、Specifier、Reviewer                              |
| `diagnosing-bugs`                | P   | 硬 bug 的诊断循环，先建反馈回路                                    | Coder、Reviewer                                         |
| `code-review`                    | P   | 双轴审查（规范 + 需求）                                            | Reviewer                                                |
| `domain-modeling`                | P   | 挑战术语、固化领域模型与 ADR                                       | Specifier、doc-maintainer、Reviewer                     |
| `grilling`                       | P   | 结构化追问，澄清歧义                                               | Specifier、Swarm Orchestrator                           |
| `writing-for-agents`             | P   | 为 agent 编写提示词与技能定义                                      | Swarm Orchestrator                                      |
| `cpp-pro`                        | C   | 惯用 C++、RAII、内存安全                                           | Coder、Reviewer                                         |
| `c-pro`                          | C   | 指针运算、内存管理、C 风格接口                                     | Coder                                                   |
| `error-handling-patterns`        | C   | 错误传播、提前返回、优雅降级                                       | Coder                                                   |
| `sharp-edges`                    | C   | 识别易错 API 与危险配置                                            | Coder                                                   |
| `find-bugs`                      | C   | 分支改动中的缺陷与质量问题                                         | Coder、Reviewer                                         |
| `performance-profiling`          | C   | 先测量、再分析、后优化                                             | Coder、Reviewer                                         |
| `verification-before-completion` | C   | 证据先于断言                                                       | Coder、Reviewer                                         |
| `codebase-cleanup-tech-debt`     | C   | 识别并量化局部技术债                                               | Coder                                                   |
| `architecture`                   | C   | 架构决策框架、权衡评估                                             | Reviewer                                                |
| `architecture-decision-records`  | C   | ADR 标准写法                                                       | Specifier、doc-maintainer、Reviewer、Swarm Orchestrator |
| `c4-code`                        | C   | 代码级组件边界与依赖梳理                                           | Reviewer                                                |
| `threat-modeling-expert`         | C   | 威胁建模、攻击面识别                                               | Coder、Reviewer                                         |
| `security-auditor`               | C   | 安全审计检查项                                                     | Coder                                                   |
| `protocol-reverse-engineering`   | C   | 协议字段与边界推断                                                 | Coder                                                   |
| `binary-analysis-patterns`       | C   | 崩溃定位、内存转储分析                                             | Coder                                                   |
| `game-development`               | C   | 游戏开发编排（含 `multiplayer`、`game-design`、`pc-games` 子技能） | Reviewer、game-design                                   |
| `brainstorming`                  | C   | 把模糊想法转化为可比较方案                                         | game-design                                             |
| `scientific-critical-thinking`   | C   | 证据等级评估                                                       | game-design                                             |
| `statistical-analysis`           | C   | 样本量与判读方法                                                   | game-design                                             |
| `markdown-mermaid-writing`       | C   | Markdown 与 Mermaid 规范、文档模板                                 | doc-maintainer、Reviewer                                |
| `documentation-templates`        | C   | README / API 文档结构约定                                          | doc-maintainer                                          |
| `plan-writing`                   | C   | 任务拆解、依赖排序、验证判据                                       | Swarm Orchestrator                                      |
| `simplify-codebase`              | S   | 代码简化与死代码清理（独立学科）                                   | Coder                                                   |
| `ste-zh`                         | Z   | **报告写法**：关键信息先行、一句一事、固定状态词、如实写验证       | 全部角色（强制技能）                                    |

> `game-development` 为编排技能，`multiplayer`、`game-design`、`pc-games` 是其子技能；顶层联接已包含子技能，**不得在源仓库内创建嵌套联接**。

### 已刻意排除的技能

- `systematic-debugging`、`debugging-strategies`、`code-review-checklist`、`concise-planning`、`docs-architect`、`mermaid-expert`、`c4-architecture-c4-architecture`：与已链接的 plugin / catalog 技能**职责重叠**，按"同一主题优先 plugin 仓库"和"避免重叠技能"规则不重复链接
- `tdd`：本仓库验证顺序为编译→启动→功能→回归，且 `tests/` 为轻量自研 harness，无 C++ 单元测试框架；引入 TDD 会与本仓库既定验证流程冲突
- `agent-orchestration-improve-agent`、`agent-orchestration-multi-agent-optimize`：面向多智能体**运行时**的性能优化，本仓库 swarm 是提示词编排，无守护进程或并发运行时
- `legacy-modernizer`：倾向大范围现代化改造，与本项目"保持 ocgcore 上游兼容性"直接冲突
- 安全、渗透、提权、破坏性操作类技能：**默认不链接**，即使项目有网络层与 Lua 沙箱
