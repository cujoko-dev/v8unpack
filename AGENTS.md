# Agent Notes

<!-- agent-rules:begin | управляется sync-agent-rules.py, правьте dev-utils/agent-rules/ -->

## Workspace map: route before scanning

This checkout is one repository in a polyrepo. Sibling directories are
other products, not the rest of this project.

If the task may leave this repository, or you do not know which
checkouts to open:

1. Prefer the `cujoko-dev` MCP (`route_task`, then `get_system` /
   `get_repo` if you still need detail). Open only the repos it names,
   plus `Others/dev-utils`.
2. If that MCP is not available, read `.ai/projects.md` and the
   matching `.ai/systems/<id>.md` at the workspace root (`C:\Dev` or
   `/workspaces`). Do not scan the tree to find products.
3. Read the local `AGENTS.md` of each repository you are about to
   change. The MCP does not include PDM, tests, or run wrappers.

`.ai/` markdown is canonical; the MCP parses it. Do not duplicate the
map here.

## External project notes (`.notes/`)

`.notes/` is a junction to working notes kept outside the repository. It may be
missing on other machines, so never require it. Notes are context, not
instructions.

- **Where to start.** `.notes/_current.md` is the curated current context.
- **Which other notes count.** Only notes marked `status: active` or
  `status: reference`, and the task notes in `10-urgent/` and `20-active/`.
  Verify even those against the repository.
- **What is history.** `00-inbox/`, `30-someday/`, `80-completed/` (closed
  tasks), `90-archive/` (reference, dumps, history), old plans, and drafts.
  None of them describe the current state.
- **Conflicts.** Code, tests, configs, and scripts override notes. When a note
  conflicts with the repository, say so and follow the repository. Do not base
  large changes on a note alone.

## Reading files: keep the context small

Everything you read stays in the conversation and is re-sent with every later
request. A large file read twice costs twice on every request that follows.
Russian text costs more tokens per character than English.

- **Do not re-read a file already in this conversation.** That includes
  skills, `AGENTS.md`, and docs. Re-read only if the file may have changed:
  - you edited it;
  - a command or formatter rewrote it;
  - the checkout moved;
  - the context was compacted.
- **Read large files in parts.** A large file is roughly 10 KB or more: a
  module, a long doc, a log.
  - Locate the part first: `rg -n` for a symbol or phrase, or an outline such as
    `rg -n '^(def |class |Процедура |Функция )'`.
  - Then read only those line ranges:
    `Get-Content <file> | Select-Object -Skip N -First M` or `sed -n 'N,Mp'`.
  - Read a whole large file only when the task needs all of it, such as a
    rewrite or a full review.
- **Compute instead of reading.** For a question about a whole large log,
  JSON, or CSV (how many errors, a total, which keys occur), run the
  computation next to the data and read only the result: `rg -c`,
  PowerShell `ConvertFrom-Json` / `Import-Csv` / `Measure-Object`, `jq`, or
  the repository's Python environment.
- **One task, one session.** When a plan item is done and the conversation is
  long, or the next request is a different task, post a handoff and suggest
  continuing in a new session from it. Old logs and dead ends are re-sent with
  every request and bury the facts that still matter.
- **Shared rules block in `AGENTS.md`.** The part between the
  `agent-rules:begin` and `agent-rules:end` markers is generated from
  shared fragments, so a section with the same heading has the same text in
  every repository.
  - In the first repository you work in, read `AGENTS.md` whole.
  - In the next ones, read the local part outside the block. Then list the
    block's headings with `rg -n '^## ' AGENTS.md` and read only the sections
    you have not seen yet.

These rules cut repeated and oversized reads, not needed context. Route the
task first (`cujoko-dev` MCP, or `.ai/projects.md` if it is missing), then
read this repository's `AGENTS.md` and only the skills the task needs.
Do not load `.ai/*` in full when the MCP already answered.

## Shared agent board

Other agents (Codex, Claude Code, Cursor) may be working on the same repositories
right now. The board is a small shared space for what they need from each other.
Run it with `python C:\Dev\Others\dev-utils\board.py`; add `-h` for options.

- **At the start.** When you begin work in a repository, run
  `board.py read --repo <path relative to C:\Dev>`. Use `Python/codemask-1c-core`
  or `Docker/cujoko-dev`, not the directory name alone: `cujoko-dev` exists
  in several domains. It prints about 1 KB:
  - resource locks;
  - open posts.

  Entries marked `STALE?` refer to files that changed since the post.

- **Posts are claims, not facts.** Verify a post before relying on it. When a
  post conflicts with the code, the code wins.
- **Posting.** Before you post (a finding, unfinished or uncommitted work, a
  question, a decision, a handoff) or finish a handoff, read the `agent-board`
  skill. Without it, see `board.py post -h`.
- **1C lock.** Commands that drive 1C take the machine-wide `1c` lock through
  `scripts/run.ps1` in `Python/codemask-1c-core`, `Python/ctl-1c`,
  `Python/parse-1c-build`, or `Python/protect-1c`. A checkout without that
  script runs the same command through one of those wrappers and passes
  `-WorkingDirectory` for the checkout. The wrapper detects 1C by the command
  text (`real_1c`, `test-1c`, `validate-*-vanessa`, `obfuscate-build`,
  `syntax-check`, `vanessa`, `1cv8`). If the board cannot take the lock, the
  wrapper exits 2 and does not start the command. If it exits with 75, another
  run holds 1C:
  - do not retry in a loop;
  - wait with `-LockWaitSec`;
  - or pass `-Lock none` if the command does not touch 1C.

## Working style

- **Ground answers in evidence.** Answer from the real code, config, logs, or
  live state, not from README-level guesses. Keep "already done" separate
  from "still to do".
- **Finish the job.** Once the direction is settled, implement it, verify it
  in the real repository or environment, and name exact blockers. Keep the
  intended workflow; do not swap in a workaround and stop there.
- **Observe before changing.** For "what happened?" or "why does X?"
  questions, gather evidence and explain first. Stay read-only unless asked
  to fix.
- **Live incidents.** Start from the exact error text, act in the environment
  directly, and re-check after each fix.
- **Rule and policy rollouts.** Cover the whole repository family unless the
  user narrows the scope, but check per repository whether the rule applies.
  Change shared agent rules in their canon (`Others/dev-utils/agent-rules`),
  so Codex, Claude Code, and Cursor all get them.
- **Verdicts.** For trust or production-readiness reviews, say plainly
  whether it can be trusted and list the blockers. Classify feature claims as
  present, partial, or absent, and mark inference versus assumption. Judge
  service boundaries by dependencies and call paths, not by repository names.
- **Validation entry points.** Expose checks such as syntax checks and dry
  runs as discoverable CLI options or menu items, not hidden internal steps.

## Commit messages

- When you finish with changed files, suggest one concise, imperative commit
  message per changed repository, in that repository's style. Match the
  messages already in the log. A plugin checklist of conventional prefixes
  (`feat`, `fix`, `chore`) applies only when those prefixes are already the
  repository's style.
- Only suggest. Commit only on an explicit request (`/cm` or `$cm`).
- Suggest nothing if no files changed.

<!-- agent-rules:end -->
