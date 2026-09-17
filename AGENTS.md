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
  - Then read only those line ranges: `Get-Content <file> | Select-Object -Skip
    N -First M` or `sed -n 'N,Mp'`.
  - Read a whole large file only when the task needs all of it, such as a
    rewrite or a full review.
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

  Repeat `--repo` on `post` when the note belongs to several checkouts. Use
  `*` only when it is truly workspace-wide.

  Entries marked `STALE?` refer to files that changed since the post.

- **Posts are claims, not facts.** Verify a post before relying on it. When a
  post conflicts with the code, the code wins.
- **What to post.** Post only what another agent would otherwise rediscover or
  collide with. Keep a post under 1500 characters, and give evidence (a
  command, log, or commit).
  - `finding`: a non-obvious fact. Pass `--paths` with the files it depends on,
    so the post goes stale when they change.
  - `status`: work you leave unfinished or uncommitted.
  - `question`: something you are blocked on.
  - `decision`: a choice other agents must follow. Use `--supersedes <id>` to
    replace an older post.
  - `handoff`: multi-step work another session should continue. The handoff
    text lives on the board, not in `.notes/`. Write it to a temp file,
    starting with a summary. Post it with `--body-file` (up to 30000
    characters). It stays open until someone resolves it; the next agent reads
    it with `board.py show <id>`.
- **Who writes.** Pass `--author` with your tool name. Resolve your own posts
  with `board.py resolve <id>` once they no longer hold.
- **Finishing a handoff.** When you finish the work it hands over:
  1. Move what must outlast the task into the project's `.notes/_current.md` or
     the repository docs. That means the current state, decisions, and open
     problems, not the history of steps.
  2. Resolve the handoff.

  If you finish only part, post an updated handoff with `--supersedes <id>`.

- **1C lock.** `scripts/run.ps1` takes the machine-wide `1c` lock on its own for
  commands that drive 1C. If it exits with 75, another run holds 1C:
  - do not retry in a loop;
  - wait with `-LockWaitSec`;
  - or pass `-Lock none` if the command does not touch 1C.

## Commit messages

- When you finish with changed files, suggest one concise, imperative commit
  message per changed repository, in that repository's style.
- Only suggest. Commit only on an explicit request (`/cm` or `$cm`).
- Suggest nothing if no files changed.

<!-- agent-rules:end -->
