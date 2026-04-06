---
description: "Use when editing DVDStyler C/C++ source or headers. Covers wxWidgets-centric patterns, naming, logging, platform guards, and minimal-change expectations for this codebase."
name: "DVDStyler C/C++ Preferences"
applyTo: "**/*.{c,cc,cpp,cxx,h,hpp,hxx}"
---
# DVDStyler C/C++ Preferences

- Treat these as repo-specific preferences for C/C++ changes unless the surrounding file clearly establishes a different local pattern.
- Preserve existing style and keep diffs narrow. Do not reformat unrelated code or rename symbols just for consistency.
- Match the repo's established formatting: 4-space indentation, same-line opening braces, and the local whitespace conventions of the file you are editing.
- Follow the exact local naming patterns already used in the touched file or class. Keep PascalCase for classes and types where that is established, preserve whether methods and functions use wx-style names or lower camel case, and keep `m_` for instance members and `s_` for statics where present.
- Prefer `wxString` and established wxWidgets types and APIs over introducing new string or UI abstractions unless there is a clear existing precedent in the touched area.
- Use wxWidgets logging and error-reporting patterns such as `wxLogError`, `wxLogWarning`, and boolean/error-code based control flow instead of introducing exception-heavy handling into code that does not already use it.
- Respect platform-specific code paths and feature guards such as `__WXMSW__`, `__WXMAC__`, and optional library guards. Extend the existing conditional structure rather than replacing it.
- Keep compatibility with the repo's existing build setup and C++11 baseline. Avoid newer language or library features unless the relevant code already depends on them.
- When adding comments, prefer short comments or existing Doxygen-style documentation patterns used near the touched declarations. Do not add noisy explanatory comments.
- Before proposing framework-level refactors, new third-party dependencies, or wide API changes, first prefer the smallest change that fits the current architecture based on wxWidgets, Autotools, and the existing media toolchain integration.