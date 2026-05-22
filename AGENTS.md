# DVDStyler Agent Guide

This file helps AI coding agents become productive quickly in this repository.

DVDStyler is a cross-platform desktop application and should be treated as such when making changes.

## Scope And Priority

- Use this file as the primary project-wide guide for workflow and navigation.
- For C/C++ edits, also follow `.github/instructions/dvdstyler-cpp.instructions.md`.
- Keep changes minimal and targeted. Do not reformat unrelated files.

## Build And Run

- Start with platform docs:
  - [BUILD.md](BUILD.md)
  - [BUILD-MACOS-HOMEBREW.md](BUILD-MACOS-HOMEBREW.md)
  - [BUILD-LINUX.md](BUILD-LINUX.md)
- Assume Linux, Windows, and macOS are first-class targets when planning or reviewing changes.
- Typical macOS Homebrew flow:
  1. `./autogen.sh`
  2. `./configure`
  3. `make -j$(sysctl -n hw.ncpu)`
  4. `sudo make DVDStyler.app`
  5. `sudo make libs`
- When packaging macOS libs, ensure dylibbundler can see these paths used by the project makefiles:
  - `/opt/homebrew/lib`
  - `/opt/homebrew/opt/libarchive/lib`
  - `/usr/local/lib`
  - `/opt/local/lib`

## Testing Reality

- No dedicated automated test suite is defined in this repository.
- Validate by building the touched target and checking for compile/link errors.
- Ideally validate changes on Linux, Windows, and macOS, or clearly call out platform coverage gaps when full tri-platform validation is not possible.
- If behavior changes are user-facing, describe manual verification steps in your response.

## TODO

- Refine and document Windows-specific build instructions with a dedicated, reproducible workflow comparable to the Linux and macOS guides.

## Project Layout

- `src/`: main wxWidgets application, DVD model, process pipeline, dialogs.
- `wxVillaLib/`: supporting static library used by the app.
- `data/`, `docs/`, `locale/`, `templates/`, `transitions/`: runtime assets and documentation.

## Architecture Notes

- The app is a wxWidgets desktop program with process-oriented authoring/burn pipeline classes in `src/Process*.cpp`.
- Parser components are generated from `src/dvdvml.l` and `src/dvdvmy.y`.
- Keep compatibility with existing autotools-based build flow and C++11 expectations.

## Change Hygiene

- Before edits and before finalizing, check git status to avoid touching unrelated changes.
- Do not use destructive git operations unless explicitly requested by the user.
- When uncertain about cross-file impact, prefer smaller incremental edits and report assumptions.

## Additional Context

- [README.md](README.md) explains this fork and current goals.
- [README](README) contains legacy upstream project context.
- [ChangeLog](ChangeLog) captures historical behavior and release notes.
