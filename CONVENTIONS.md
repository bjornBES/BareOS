# BareOS Conventions

Living document. If a rule below stops matching reality, fix the rule, don't
just violate it quietly. Anything not covered here: match the nearest
existing file, then add the rule.

---

## 1. File naming / header ownership

Every subsystem (`signal`, `proc`, `fs`, `vmm`, ...) follows this file set.
Not every subsystem needs all of them, but if a file exists, it goes in the
slot below — no ad hoc `*_types.h` duplicates.

| File | Contains | Included by |
|---|---|---|
| `<subsys>_uapi.h` | POSIX/ABI-visible types ONLY (`sigaction_t`, `siginfo_t`, struct layouts crossing the syscall boundary). Single source of truth — never redeclared elsewhere. | kernel + libbareOS + musl-facing shim |
| `<subsys>.h` | Kernel-internal public API (function decls) for the subsystem. Generic, non-arch. | other kernel subsystems |
| `<subsys>_sys.h` / `<subsys>_sys.c` | Syscall entry points (the `sys_*` functions the syscall table X-macro points to). | syscall dispatch only |
| `arch/<arch>/<subsys>_arch.h` / `.c` | Arch-specific implementation. Never declares a type generic code or userspace needs — if it needs to be visible outside the arch dir, it belongs in `<subsys>.h` or `<subsys>_uapi.h` instead. | generic `<subsys>.c` only, via arch dispatch |
| `<subsys>_asm.asm` | Arch-specific asm (trampolines, context save/restore, etc). | linked in, not `#include`d |

Rule of thumb when unsure where a type goes:
- Does userspace (musl, libbareOS) need to see it? → `_uapi.h`
- Is it kernel-wide but not arch-specific? → `<subsys>.h`
- Is it only meaningful on one arch? → `arch/<arch>/<subsys>_arch.h`

No monolithic `arch.h`. `*_types.h` stays as a valid file — exact rule for
what belongs in it vs. `_uapi.h` vs. `.h` is still TBD (see open questions
at bottom). Until decided, default to the table above and treat
`_types.h` as an exception you'll define later.

---

## 2. Build / tooling

- `compile_commands.json` generated via `bear -- make` (or `compiledb`) —
  regenerate any time Makefile include paths change. This is what
  `.vscode/c_cpp_properties.json` points at (`compileCommands`, not
  `includePath`). Never hand-maintain a merged include path list again.
- `.clang-format` governs all C/C++; run before commit, not after.
- Config system: `config/config.env` (`key = value`) → `gen_config.sh`
  generates `config.h` (`#pragma once`) and `config.inc` (NASM `%define`)
  in one pass. Never hand-edit generated output.

---

## 3. Naming conventions (code)

- Arch-specific functions: `subsystem_arch_verb()` (e.g. `vmm_arch_map()`,
  `signal_arch_dispatch()`). Never bury arch logic under a generic name.
- Return convention: internal kernel functions return either `0` (success)
  or a positive errno code (failure) — that's the entire contract, no
  separate `RETURN_GOOD`/`RETURN_FAILED` sentinel values. Syscalls
  translate this to POSIX `-errno` at the ABI boundary on the way out.
- Address space constants: centralized in `memdefs.h`, not re-`#define`d
  per file.
- Assembly: Intel syntax, always.
- Header guards: `#pragma once` only. Never `#ifndef`/`#define`/`#endif`
  guards, anywhere, no exceptions.

---

## 4. Data structure patterns

- **Intrusive linked lists** by default — embed the list node in the
  struct, no separate allocation. Use `container_of` for recovery; a
  direct cast is only allowed when the node is documented (comment) as
  guaranteed-first-member.
- A thread/task lives in exactly **one** queue at a time (run, sleep, or
  wait) — reuse a single `node` field across all three, don't give each
  queue type its own field.
- Driver-owned structs (inodes, etc.) embed the base struct
  (`inode_t base`) as the first member for safe upcast/downcast.
- Filesystem detection is probe-based — no explicit `fs_type` parameter
  threaded through mount paths.

---

## 5. ABI / cross-boundary rules

- Any struct crossing a compiler boundary (bootloader ↔ kernel, kernel ↔
  userspace) uses explicitly-sized integer types only. No `int`, `long`,
  `size_t` in ABI structs — `uint32_t`, `int64_t`, etc.
- `_uapi.h` headers are the ABI contract. Changing a field's size/order
  in one of these is a breaking change and should be treated with the
  same weight as a syscall number change.

---

## 6. Userspace libc

- Day one of userspace: **musl only**. All userspace code (own tools
  included) links against musl. There is no `libbareOS` yet.
- `libbareOS` arrives later as its own phase. Once it exists, the
  two-libc rule applies retroactively: musl and `libbareOS` never share
  headers, even for identical-looking types — duplicate deliberately
  rather than `#include` across that line. Own userproc code migrates to
  `libbareOS` at that point; third-party tools stay on musl.
- Don't build `libbareOS`-shaped abstractions prematurely while still on
  musl-only — that's scope creep from a phase that hasn't started.

## 7. Commit messages

Format: `subsystem: what changed, stated plainly`

- `bootloader_stage2: added GOP pixel format detection`
- `processes: changed fork so it copies the fd table refcounted`
- `signal: fixed sigaction_t clobbering kernel-side sigset`

Subsystem prefix matches the directory/subsystem name (or closest
sensible one). No vague messages ("fixes", "wip", "stuff") — state is
either what changed or what it now does.

## 8. plans.md / phase structure

- Each phase is scoped to a **subsystem**, not a cross-cutting feature
  set (e.g. a phase is "SMP", "AHCI", "APIC" — not "make things faster").
- Each phase gets an explicit depth target going in: basic implementation,
  full implementation, or quick-and-dirty. Decide which before starting,
  not after — this is the thing that failed with dynamic linking (no
  declared depth target, so "done" kept moving).
- A phase is finished when it hits its declared depth target, even if
  more could theoretically be added — extra polish becomes a new phase,
  not scope creep on the current one.

---

## 9. Open questions / to fill in as they come up

- [ ] Exact `_types.h` vs `_uapi.h` vs `.h` rule — currently deferred,
      see note in section 1
- [ ] Syscall X-macro conventions worth writing down explicitly here
      (currently only in your head / prior repo)
- [ ] Where phase-scoped code lives vs. "core" — does a phase get its own
      directory, or does it land directly in the subsystem's permanent
      home as it's written?
