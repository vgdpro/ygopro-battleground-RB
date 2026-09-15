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

| 文件                          | 类型 | 内容                                                   |
| ----------------------------- | ---- | ------------------------------------------------------ |
| `project-overview.md`         | 通用 | 项目定位、核心流程、高风险区域                         |
| `architecture.md`             | 通用 | 模块结构、边界、数据流、不变量                         |
| `standards.md`                | 通用 | 编码规范、设计偏好、范围规则                           |
| `testing.md`                  | 通用 | 验证顺序、测试布局、区域命令                           |
| `workflow.md`                 | 通用 | 交付流程、开发路径、发布门禁                           |
| `battleground-game-design.md` | 战旗 | **玩法设计**：核心循环、规则语义、平衡目标、待确认决策 |
| `battleground-design.md`      | 战旗 | **架构设计**：单 duel 三 field、field 分配、核心约束   |
| `battleground-roadmap.md`     | 战旗 | **任务路线图**：里程碑、任务、依赖、状态、验收标准     |
| `battleground-tech.md`        | 战旗 | **技术实现**：接口、实现文件、风险、精简验证证据       |
| `task-numbering.md`           | 流程 | **任务编号规则**：编号格式、分配规则、旧编号映射       |
| `doc-standards.md`            | 通用 | **文档编写规范**：格式、内容规则、代码对比修正规则     |

## 战旗模式知识库使用规则

- `battleground-game-design.md` 是**玩法权威**：记录玩家可感知规则、设计理由和平衡目标；未经用户确认的提案不得写成既定规则
- `battleground-design.md` 是**设计权威**：任何架构决策必须先对照此文档，修改设计文档需 Specifier 确认
- `battleground-roadmap.md` 是**任务权威**：里程碑、任务编号、依赖、状态和验收标准只在此维护
- `battleground-tech.md` 是**实现事实权威**：记录接口、实现行为、文件归属、风险和精简验证证据，不维护任务状态
- `task-numbering.md` 是**编号制度权威**：只记录编号格式、分配规则和旧编号映射，不复制任务表
- `doc-standards.md` 是**编写规范权威**：所有知识库和记忆库文件的编辑必须遵循此规范
- Swarm 维护分工：Specifier 输出任务、依赖和验收标准；Architect/Orchestrator 输出已确认架构决策；QA 输出状态、完成日期和精简验证证据；仅 Doc Maintainer 编辑知识库，并在状态显著变化时同步 `memories/`
- `memories/` 是非权威恢复摘要；发生冲突时以对应 knowledge-base 权威文件为准
- 文档格式清理和过期内容删除由 **doc-maintainer** agent 负责（斜杠 `/doc-maintainer` 调用，或编辑知识库文件时自动加载 `.github/instructions/doc-maintainer.instructions.md`)

## 本地技能库（`.agents/skills`）

- **来源**：本地技能库 `D:\SourceCode\agent-skills-hub`，通过 Windows 目录联接（junction）挂载到 `.agents/skills/<skill-name>`，**不复制文件**
- **更新**：在源仓库执行 `git pull` 后，本项目链接内容自动生效；不维护第二份副本，不生成锁定文件
- **定位**：通用方法与检查清单来源。技能与仓库规范冲突时，**一律以本仓库知识库与 instructions 为准**
- **不适用**：技能库中无 Lua 专用技能、无 SQLite 专用技能、无 C++ 单元测试框架技能；`testing-patterns` 为 Jest 专用，不得用于 `ocgcore-tests`
- **边界**：目录联接不是安全边界，不得链接密钥、`.env` 或 `skills` 目录之外的任何内容

### 按角色分配

| 角色               | 技能                                                                                                                                                                                                                    |
| ------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Coder              | `cpp-pro`、`c-pro`、`error-handling-patterns`、`systematic-debugging`、`debugging-strategies`、`find-bugs`、`performance-profiling`                                                                                     |
| Cleaner            | `cpp-pro`、`codebase-cleanup-tech-debt`、`systematic-debugging`、`verification-before-completion`                                                                                                                       |
| Architect          | `architecture`、`architecture-decision-records`、`c4-code`、`c4-architecture-c4-architecture`、`game-development/multiplayer`、`threat-modeling-expert`、`markdown-mermaid-writing`、`docs-architect`                   |
| Hardender          | `systematic-debugging`、`error-handling-patterns`、`sharp-edges`、`cpp-pro`、`threat-modeling-expert`、`security-auditor`、`protocol-reverse-engineering`、`binary-analysis-patterns`、`verification-before-completion` |
| QA                 | `verification-before-completion`、`systematic-debugging`、`code-review-checklist`、`find-bugs`、`cpp-pro`、`performance-profiling`、`game-development/multiplayer`                                                      |
| Specifier          | `plan-writing`、`concise-planning`、`architecture-decision-records`、`docs-architect`                                                                                                                                   |
| Game Designer      | `brainstorming`、`game-development/game-design`、`game-development/multiplayer`、`game-development/pc-games`                                                                                                            |
| Balance Analyst    | `scientific-critical-thinking`、`game-development/game-design`、`game-development/multiplayer`、`statistical-analysis`                                                                                                  |
| doc-maintainer     | `markdown-mermaid-writing`、`mermaid-expert`、`docs-architect`、`documentation-templates`、`architecture-decision-records`                                                                                              |
| Swarm Orchestrator | `plan-writing`、`concise-planning`、`architecture-decision-records`、`agent-orchestration-improve-agent`、`agent-orchestration-multi-agent-optimize`、`writing-skills`                                                  |

> `game-development` 为编排技能，`game-design` 与 `multiplayer` 是其子技能；顶层联接已包含子技能，**不得在源仓库内创建嵌套联接**。
