---
name: "Plan With Knowledge Base"
description: "Plan a task by reading the local project knowledge base first, then identifying the controlling code path, change scope, and validation strategy."
agent: "Reviewer"
model: "GPT-5 (copilot)"
tools: [read, search]
argument-hint: "Task or behavior to analyze"
---

Use the local `knowledge-base/` files before you inspect implementation details.

For battleground work, use `battleground-tech.md` for interfaces and known risks, and `battleground-implemented.md` for capabilities that already exist and their `M*` numbers. Use `battleground-design.md` for architecture and `battleground-game-design.md` for player-facing rules. Unimplemented work is not recorded in the knowledge base. Treat `memories/` as a non-authoritative recovery summary.

For the requested task:

1. Identify which knowledge-base files matter.
2. Inspect the nearest controlling code path.
3. State one falsifiable local hypothesis.
4. Propose the smallest safe implementation direction.
5. Recommend the narrowest validation that should run after the first edit.
