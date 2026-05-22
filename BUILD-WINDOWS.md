# How to Build DVDStyler on Windows

The recommended Windows workflow is MSYS2 with a 64-bit MinGW toolchain.

## 1 Install MSYS2

Install MSYS2 from https://www.msys2.org/ and complete the post-install update flow.

After installation, open an `MSYS2 UCRT64` shell for 64-bit builds.

## 2 Install Toolchain And Build Tools

Install MinGW-w64 C/C++ and core build tooling with pacman:

```bash
pacman -Syu
pacman -S --needed base-devel git autoconf automake libtool bison flex pkgconf gettext xmlto zip unzip
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
```

Notes:

- `mingw-w64-ucrt-x86_64-toolchain` provides GCC/G++ (C/C++ compiler toolchain).
- If you prefer the MINGW64 environment, use the corresponding `mingw-w64-x86_64-*` package names instead.

## 3 Install DVDStyler Build Dependencies

Use the package set that matches your selected shell environment.

### UCRT64 package set

Run this in an `MSYS2 UCRT64` shell:

```bash
pacman -S --needed \
	mingw-w64-ucrt-x86_64-wxwidgets3.2-msw \
	mingw-w64-ucrt-x86_64-ffmpeg \
	mingw-w64-ucrt-x86_64-libexif \
	mingw-w64-ucrt-x86_64-fontconfig \
	mingw-w64-ucrt-x86_64-cairo \
	mingw-w64-ucrt-x86_64-pango \
	dvdauthor cdrtools dvd+rw-tools ffmpeg
```

### MINGW64 package set

Run this in an `MSYS2 MINGW64` shell:

```bash
pacman -S --needed \
	mingw-w64-x86_64-wxwidgets3.2-msw \
	mingw-w64-x86_64-ffmpeg \
	mingw-w64-x86_64-libexif \
	mingw-w64-x86_64-fontconfig \
	mingw-w64-x86_64-cairo \
	mingw-w64-x86_64-pango \
	dvdauthor cdrtools dvd+rw-tools ffmpeg
```

### Optional wxSVG packages (if available)

If wxSVG is packaged in your environment, install one of:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-wxsvg
```

or

```bash
pacman -S --needed mingw-w64-x86_64-wxsvg
```

Notes:

- Package naming can vary between MSYS2 environments. Do not mix UCRT64 and MINGW64 package prefixes in the same shell.
- `libudev`, `libdbus`, and GNOME thumbnail helpers are optional in this project and may be unavailable/irrelevant on Windows.
- If `wxSVG` is not available as a package in your selected environment, build and install wxSVG separately, then ensure `pkg-config` can find `libwxsvg`.

## 4 Build DVDStyler

From the repository root in the same MinGW shell:

```bash
./autogen.sh
./configure
make -j$(nproc)
```

If `nproc` is not available in your shell, replace it with a fixed value such as `-j8`.

## 5 Validate Build Output

- Confirm `configure` detects wxWidgets, ffmpeg/libav, libexif, and fontconfig.
- Confirm binaries are produced without link errors.
- Run a basic smoke test: launch app, open a sample project, preview menu/video, and run a non-burning authoring path.

## TODO (Windows docs)

- Validate and pin exact package names for both `ucrt64` and `mingw64` environments.
- Add a known-good `./configure` invocation with explicit Windows-specific flags if needed.
- Document runtime DLL packaging strategy for distributable artifacts.
- Add troubleshooting notes for path conversion, `pkg-config`, and mixed-shell environment issues.
