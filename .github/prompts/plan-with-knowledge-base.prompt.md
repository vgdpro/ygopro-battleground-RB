---
name: "Plan With Knowledge Base"
description: "Plan a task by reading the local project knowledge base first, then identifying the controlling code path, change scope, and validation strategy."
agent: "Project Architect"
model: "GPT-5 (copilot)"
tools: [read, search]
argument-hint: "Task or behavior to analyze"
---

Use the local `knowledge-base/` files before you inspect implementation details.

For battleground tasks, read `battleground-roadmap.md` first for the current task, dependencies, status, and acceptance criteria. Use `battleground-design.md` for architecture, `battleground-tech.md` for implementation facts, and `task-numbering.md` only for numbering rules or legacy mappings. Treat `memories/` as a non-authoritative recovery summary.

For the requested task:

1. Identify which knowledge-base files matter.
2. Inspect the nearest controlling code path.
3. State one falsifiable local hypothesis.
4. Propose the smallest safe implementation direction.
5. Recommend the narrowest validation that should run after the first edit.
