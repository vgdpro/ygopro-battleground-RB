---
name: "Bootstrap Knowledge Base"
description: "Draft or refresh the local knowledge-base files by reading the current repository docs, code layout, tests, and configuration."
agent: "agent"
model: "GPT-5 (copilot)"
tools: [read, search, edit]
argument-hint: "Optional scope, such as backend only, frontend only, or docs-first"
---

Read the current repository and refresh the files under `knowledge-base/`.

Sources to prefer:

1. Existing README files
2. Docs and architecture notes
3. Build and test configuration
4. Representative source and test directories

Requirements:

- Keep each knowledge-base file concise and high signal.
- Summarize repository reality instead of copying large docs verbatim.
- Call out uncertainty when the repository does not provide enough evidence.
- Prefer practical guidance that future agents can act on.
- If a knowledge-base file already contains useful curated content, preserve and refine it instead of replacing it blindly.

Minimum output:

1. What sources you used.
2. Which knowledge-base files you updated.
3. Any critical gaps that still need human input.
