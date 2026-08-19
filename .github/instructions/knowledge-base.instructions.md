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

| 文件                     | 类型 | 内容                                                   |
| ------------------------ | ---- | ------------------------------------------------------ |
| `project-overview.md`    | 通用 | 项目定位、核心流程、高风险区域                         |
| `architecture.md`        | 通用 | 模块结构、边界、数据流、不变量                         |
| `standards.md`           | 通用 | 编码规范、设计偏好、范围规则                           |
| `testing.md`             | 通用 | 验证顺序、测试布局、区域命令                           |
| `workflow.md`            | 通用 | 交付流程、开发路径、发布门禁                           |
| `battleground-design.md` | 战旗 | **架构设计**：为什么单duel三field、field分配、核心约束 |
| `battleground-tech.md`   | 战旗 | **技术实现**：步骤分解、接口定义、改动清单、已知风险   |

## 战旗模式知识库使用规则

- `battleground-design.md` 是**设计权威**：任何架构决策必须先对照此文档，修改设计文档需 Specifier 确认
- `battleground-tech.md` 是**实现进度表**：完成每一步后由 QA 更新状态，Coder 实现前先确认前置步骤已完成
- 两个文档由 Swarm 工作流维护：Specifier 输出设计增量 → Orchestrator 写入 `battleground-design.md`；QA 验证后直接编辑 `battleground-tech.md`
- 如果代码实现与文档冲突，优先更新文档还是修改代码，由 Specifier/Architect 裁决
