---
description: "Use when planning delivery, splitting work across roles, preparing handoffs, or deciding what to validate next. Covers role discipline and concise coordination."
---

# Workflow Guidance

- Stay inside the current task scope unless the user expands it.
- When splitting work across agents, preserve a clear chain: planning, implementation, review, verification.
- Summaries and handoffs should be short, actionable, and specific about next steps.
- When a role completes work, pass along what changed, how it was validated, and any open risk.
- If a repository defines its own validation order or release gates, follow `knowledge-base/testing.md`.

## Grill Me 模式

当用户说 **"grill me"**，或任务在范围、风险、验收标准上存在歧义时，**先**用 `vscode_askQuestions` 一次性提出结构化追问，**再**开始工作。按角色聚焦：

- **范围**：精确边界、明确排除项、P0/P1 裁剪
- **验收**：如何判定完成、最窄可工作的验证、硬性要求
- **隐含约束**：性能、协议兼容、安全、代码风格、与既有设计的冲突
- **风险偏好**：最小改动 vs 更健壮、是否允许跨模块
- **实施路径**：期望落在哪一层（配置 / Lua / gframe / ocgcore）、是否预留扩展点

首次接触该用户时抛出完整问题集；后续同类任务只问变化部分。各 agent 只需说明**自己的触发条件**，问题清单不重复。

## 技能使用

技能清单与角色映射以 `.github/instructions/knowledge-base.instructions.md` 为唯一权威。agent 定义不复制技能表，只列本角色**必须**使用的少数技能。
