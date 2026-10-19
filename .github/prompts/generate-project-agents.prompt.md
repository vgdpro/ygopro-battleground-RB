---
name: "Generate Project Agents"
description: "Generate or refresh only the fixed core agent set from the local knowledge base and current codebase, adding repository-specific constraints to those roles."
agent: "agent"
model: "GPT-5 (copilot)"
tools: [read, search, edit]
argument-hint: "Optional focus, such as backend API, mobile app, data pipeline, or test-heavy workflow"
---

Read the repository knowledge base in `knowledge-base/` and inspect the current codebase shape.

Then generate or refresh only these custom agents under `.github/agents/`:

- `swarm-orchestrator.agent.md`
- `specifier.agent.md`
- `coder.agent.md`
- `architect.agent.md`
- `qa.agent.md`
- `doc-maintainer.agent.md`
- `game-design.agent.md` (on-demand, game design and balance only)

Requirements:

- Do not invent new core roles or replace these with module-specialist roles.
- `Coder` owns implementation, cleanup, and hardening as three stages inside one task; do not split them into separate agents.
- `game-design` merges gameplay design and balance review; it never writes code.
- Add repository-specific constraints to each role, but do not copy build commands or language rules into every agent: they live in `.github/instructions/engineering.instructions.md`, and the skill-role mapping lives only in `.github/instructions/knowledge-base.instructions.md`.
- Reuse existing agent files if they are already close to correct.
- Keep descriptions keyword-rich so Copilot can discover them.
- Ensure the agents reflect the real architecture, testing workflow, and delivery process from `knowledge-base/`.
- For battleground work, generated agents must treat `battleground-tech.md` as the interface/risk source and `battleground-implemented.md` as the completed-capability and `M*` numbering source. Unimplemented work is never recorded.
- Treat `memories/` as non-authoritative recovery context. QA is read-only and hands off proposed status, milestone completion dates, and evidence; only Doc Maintainer edits `knowledge-base/` or `memories/`.
- If the knowledge base is incomplete, keep the supplementation conservative and call out what is missing.

Minimum output:

1. A short summary of the repository shape you inferred.
2. The repository-specific constraints you added to each of the six roles.
3. The list of agent files created or updated.
4. The actual `.github/agents/*.agent.md` file changes.
