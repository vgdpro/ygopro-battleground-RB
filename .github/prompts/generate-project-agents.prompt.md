---
name: "Generate Project Agents"
description: "Generate or refresh only the fixed six-pack swarm-forge agents from the local knowledge base and current codebase, adding repository-specific constraints to those roles."
agent: "agent"
model: "GPT-5 (copilot)"
tools: [read, search, edit]
argument-hint: "Optional focus, such as backend API, mobile app, data pipeline, or test-heavy workflow"
---

Read the repository knowledge base in `knowledge-base/` and inspect the current codebase shape.

Then generate or refresh only these six custom agents under `.github/agents/`:

- `specifier.agent.md`
- `coder.agent.md`
- `cleaner.agent.md`
- `architect.agent.md`
- `hardender.agent.md`
- `qa.agent.md`

Requirements:

- Do not invent new core roles or replace these six with module-specialist roles.
- Preserve the fixed six-pack responsibilities from swarm-forge.
- Add repository-specific constraints to each role, such as language rules, subsystem focus, build commands, testing requirements, and workflow expectations.
- Reuse existing agent files if they are already close to correct.
- Keep descriptions keyword-rich so Copilot can discover them.
- Ensure the agents reflect the real architecture, testing workflow, and delivery process from `knowledge-base/`.
- If the knowledge base is incomplete, keep the supplementation conservative and call out what is missing.

Minimum output:

1. A short summary of the repository shape you inferred.
2. The repository-specific constraints you added to each of the six roles.
3. The list of agent files created or updated.
4. The actual `.github/agents/*.agent.md` file changes.
