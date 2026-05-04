# How to Build DVDStyler on MacOS using MacPorts

DVD Styler is an exceptionally difficult application to build, especially on MacOS. There are not great instructions out there for doing this, so I decided to compile my own.

## 1 Install Prerequisites

```bash
sudo port install wx-widgets-3.2
```

(maybe libtool autogen automake autoconf wxwidgets-3.2 ffmpeg_devel)

## 2 Build wxSVG

The version of wxsvg in MacPorts (1.5.22) is too old for DVDStyler, so you will need to build from source following the instructions below.

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
./configure --with-wxdir=/opt/local/Library/Frameworks/wxWidgets.framework/Versions/wxWidgets/3.0/bin
make -j$(sysctl -n hw.ncpu)
sudo make DVDStyler.app
sudo make libs
```

## References

<https://docs.wxwidgets.org/3.3/plat_osx_install.html>

<https://sourceforge.net/p/dvdstyler/discussion/318795/thread/d7442b04/>

<https://sourceforge.net/p/dvdstyler/discussion/318795/thread/4c443677/>
