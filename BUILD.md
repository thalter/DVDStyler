# How to Build DVDStyler on MacOS

DVD Styler is an exceptionally difficult application to build, especially on MacOS. There are not great instructions out there for doing this, so I decided to compile my own.

## Notes

## 1 Install Prerequisites

You will need [Homebrew](https://brew.sh/) and Xcode to follow this procedure. Xcode can be installed via the App Store, and instructions to install homebrew are at the homebrew site.

## 2 Create a symlink to /opt/local

This will allow `make` to pull in external libraries from your homebrew cellar.

```bash
sudo ln -s /opt/homebrew /opt/local
```

## 3 Install Prerequisites using Homebrew

### 3.1 Install Homebrew Packages

```bash
brew install ffmpeg ffmpeg-full dvdauthor autoconf automake libtool libconfig libexif libsvg-cairo xmlto cdrtools mjpegtools dylibbundler pkgconf wxwidgets pango
```

### 3.2 Make ffmpeg-full take precedence over ffmpeg

Package `ffmpeg` is a scaled-down distribution of ffmpeg, and is incomplete for our purposes, so we will need `ffmpeg-full` instead. Because ffmpeg-full is an alternate version of an existing homebrew formula, it is not simlinked into /opt/homebrew by default. Take the following steps to correct this.

```bash
echo 'export PATH="/opt/homebrew/opt/ffmpeg-full/bin:$PATH"' >> ~/.zshrc
```

Add the following lines to your .zshrc/.bashrc file to allow `make` to find ffmpeg-full

```bash
export LDFLAGS="-L/opt/homebrew/opt/ffmpeg-full/lib"
export CPPFLAGS="-I/opt/homebrew/opt/ffmpeg-full/include"
export PKG_CONFIG_PATH="/opt/homebrew/opt/ffmpeg-full/lib/pkgconfig"
```

You can either  `source` your updated rc file, or just launch a new terminal window to pick up the new environment variables.

## 4 Download and build wxSVG

wxSVG is not available as a prebuilt binary, so you will need to build it from source, following the instructions below.

### 4.1 Clone Repository

```bash
git clone https://git.code.sf.net/p/wxsvg/git wxsvg-git
cd wxsvg-git
```

### 4.2 File Modifications

Note that you will need to manually modify the following files in order for the build to be successful.

#### configure.ac

Replace line 138 (after # check libexif) with the line below

```m4
PKG_CHECK_MODULES(LIBEXIF, libexif, [CXXFLAGS="$CXXFLAGS $LIBEXIF_CFLAGS"  LIBS="$LIBS $LIBEXIF_LIBS"], [AC_MSG_ERROR([*** missing libexif ***])])
```

#### src/cairo/SVGCanvasCairo.h, src/cairo/SVGCanvasImageCairo.h, src/cairo/SVGCanvasPathCairo.h

- Replace all instances of `#include <cairo/cairo.h>` with `#include <cairo.h>`

#### src/cairo/SVGCanvasTextCairo.cpp

- Replace all instances of `#include <cairo/cairo.h>` with `#include <cairo.h>`
- Replace all instances of `#include <cairo/cairo-quartz.h>` with `#include <cairo-quartz.h>`

### 4.3 Finish building wxSVG

```bash
./autogen.sh
./configure
make -j$(sysctl -n hw.ncpu)
sudo make install
```

## 5 Build DVDStyler

Clone this repo.

```bash
./autogen.sh
./configure
make -j$(sysctl -n hw.ncpu)
sudo make DVDStyler.app
sudo make libs
```

## References

<https://docs.wxwidgets.org/3.3/plat_osx_install.html>

<https://sourceforge.net/p/dvdstyler/discussion/318795/thread/d7442b04/>

<https://sourceforge.net/p/dvdstyler/discussion/318795/thread/4c443677/>
