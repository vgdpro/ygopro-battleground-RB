---
name: "Plan With Knowledge Base"
description: "Plan a task by reading the local project knowledge base first, then identifying the controlling code path, change scope, and validation strategy."
agent: "Project Architect"
model: "GPT-5 (copilot)"
tools: [read, search]
argument-hint: "Task or behavior to analyze"
---

Use the local `knowledge-base/` files before you inspect implementation details.

For the requested task:

1. Identify which knowledge-base files matter.
2. Inspect the nearest controlling code path.
3. State one falsifiable local hypothesis.
4. Propose the smallest safe implementation direction.
5. Recommend the narrowest validation that should run after the first edit.
