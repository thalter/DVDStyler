# How to Build DVDStyler on MacOS using MacPorts

> [!WARNING] The MacPorts version of mjpegtools is broken and does not compile, so this procedure is currently broken. Use the Homebrew instructions instead.


DVD Styler is an exceptionally difficult application to build, especially on MacOS. There are not great instructions out there for doing this, so I decided to compile my own.

## 1 Install Prerequisites

```bash
sudo port -N install cctools wxwidgets-3.2 libtool autogen automake autoconf cairo libexif ffmpeg-devel xmlto dvdauthor cdrtools libmpeg2 mjpegtools dylibbundler
```

## 2 Build wxSVG

The version of wxsvg in MacPorts (1.5.22) is too old for DVDStyler, so you will need to build from source following the instructions below.

### 4.1 Clone Repository

```bash
git clone https://git.code.sf.net/p/wxsvg/git wxsvg-git
cd wxsvg-git
```

### 4.3 Finish building wxSVG

```bash
./autogen.sh
./configure --prefix=/opt/local --with-wxdir=/opt/local/Library/Frameworks/wxWidgets.framework/Versions/wxWidgets/3.1/bin
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
