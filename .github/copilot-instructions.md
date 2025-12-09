# ToyFrameV Copilot Instructions

## Project Overview

ToyFrameV is a lightweight cross-platform graphics framework based on LLGL, supporting Desktop (Windows) and Web (WebAssembly + WebGL) platforms.

**Key Directories:**
- `include/ToyFrameV/` - Public headers
- `src/` - Implementation files
- `samples/` - Sample applications
- `roadmap/` - Development roadmap
- `scripts/` - Build scripts

**Commit Message Format:** Conventional Commits
```
type(scope): description

[optional body]
```

**Types:** `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `build`, `ci`, `chore`

**Scopes:** `core`, `graphics`, `platform`, `system`, `samples`, `docs`, `build`, `web`, `windows`

---

## Slash Commands

### /commit

Analyze staged changes and commit with a properly formatted message.

**Workflow:**
1. Run `git diff --staged` to see staged changes
2. Analyze the changes to determine:
   - Type: `feat` (new feature), `fix` (bug fix), `docs` (documentation), `refactor`, `build`, etc.
   - Scope: Based on changed files (`core`, `graphics`, `platform`, `system`, `samples`, `docs`, `build`, `web`, `windows`)
   - Description: Concise summary of changes
3. Generate commit message in format: `type(scope): description`
4. If changes span multiple areas, add bullet points in body
5. Execute: `git add -A && git commit -m "message"`

**Examples:**
```
feat(core): add ThreadPool worker count configuration
fix(graphics): resolve WebGL shader compilation error
docs(roadmap): update Stage 7 completion status
build(web): fix Emscripten SDK path detection
```

---

### /sync

Synchronize local repository with remote.

**Workflow:**
1. Run `git fetch origin` to check remote status
2. Run `git status` to check local status
3. If local has uncommitted changes, prompt user to commit or stash first
4. Run `git pull --rebase origin master` to pull remote changes
5. If rebase conflicts occur, show conflict files and provide resolution guidance
6. Run `git push origin master` to push local commits
7. Report sync status: commits pulled, commits pushed, or "Already up to date"

**Error Handling:**
- Uncommitted changes: Suggest `/commit` or `git stash`
- Merge conflicts: List conflicting files, suggest manual resolution
- Push rejected: Suggest `git pull --rebase` first

---

### /read_roadmap

Analyze current implementation progress against the roadmap.

**Workflow:**
1. Read `roadmap/cross_platform_graphics_framework.md`
2. Parse all stages and their checkbox items (`[x]` = completed, `[ ]` = pending)
3. Cross-reference with actual codebase:
   - Check if files mentioned in roadmap exist
   - Verify implementations match descriptions
4. Generate progress report:

```
## 📊 ToyFrameV Roadmap Progress

### ✅ Completed Stages
- Stage 1: Project Initialization (100%)
- Stage 2: Window Creation (100%)
- ...

### 🚧 Current Stage
- Stage 8: Debug Systems
  - [ ] ConsoleSystem UI
  - [ ] Command System
  - ...

### 📋 Upcoming
- Stage 9: System Architecture Enhancement
- Stage 10: Cross-Platform Extension

### 📈 Overall Progress: X/Y tasks (Z%)
```

---

### /update_roadmap

Update roadmap file based on current implementation status.

**Workflow:**
1. Read `roadmap/cross_platform_graphics_framework.md`
2. Scan codebase for implementations mentioned in roadmap
3. For each checkbox item:
   - If implementation exists and works: change `[ ]` to `[x]`
   - If implementation removed: change `[x]` to `[ ]`
4. Update `*Last updated: ...` date at bottom of file to current date
5. Show diff preview of changes
6. Apply changes to file directly

**Verification Checks:**
- File exists: Check `include/`, `src/`, `samples/` directories
- Class/function exists: Search for declarations
- Sample works: Check if sample builds successfully

**Date Format:** `*Last updated: Month DD, YYYY*` (e.g., `*Last updated: December 9, 2025*`)

---

## Code Style Guidelines

- **C++**: Modern C++17, use `auto` sparingly, prefer explicit types
- **Headers**: Use `#pragma once`, include what you use
- **Naming**: `PascalCase` for classes, `camelCase` for methods, `UPPER_CASE` for macros
- **Comments**: Doxygen-style for public APIs
- **Platform code**: Use `Platform/` directory structure, no `#ifdef` in headers

## Build Commands

```powershell
# Windows
.\scripts\windows\build.ps1 -Target HelloTriangle

# Web
.\scripts\windows\build_web.ps1 -Target HelloTriangle

# Clean rebuild
.\scripts\windows\build.ps1 -Clean
.\scripts\windows\build_web.ps1 -Clean
```
