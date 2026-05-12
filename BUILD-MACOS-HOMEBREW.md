# How to Build DVDStyler on MacOS using Homebrew

## 1 Install Prerequisites

You will need [Homebrew](https://brew.sh/) and Xcode to follow this procedure. Xcode can be installed via the App Store, and instructions to install homebrew are at the homebrew site.

## 2 Create a symlink to /opt/local

This will allow `make` to pull in external libraries from your homebrew cellar.

```bash
sudo ln -s /opt/homebrew /opt/local
```

## 3 Install Prerequisites using Homebrew

### 3.1 Remove ffmpeg

Package `ffmpeg` is a scaled-down distribution of ffmpeg, and is incomplete for our purposes, so we will need `ffmpeg-full` instead. Both packages cannot easily exist side-by-side, so execute the following command to remove ffmpeg from your system.

```bash
brew remove ffmpeg
```

### 3.2 Install Homebrew Packages

```bash
brew install ffmpeg-full dvdauthor autoconf automake libtool libconfig libexif libsvg-cairo xmlto cdrtools mjpegtools dylibbundler pkgconf wxwidgets pango
```

## 4 Download and build wxSVG

wxSVG is not available as a prebuilt binary in Homebrew, so you will need to build it from source, following the instructions below.

### 4.1 Clone Repository

```bash
git clone https://git.code.sf.net/p/wxsvg/git wxsvg-git
cd wxsvg-git
```

## 3.2 Build and Install wxSVG

```bash
./autogen.sh
CPPFLAGS='-I/opt/homebrew/include' ./configure
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

## Troubshooting

If you run into any issues, it likely has to do with the expected locations of the external libraries. Make sure you created the simlink in step 2 above.

If you are running into issues with ffmpeg (or any of its subcomponents), make sure you don't have both `ffmpeg` and `ffmpeg-full` installed. You may need to create some additional simlinks on your system if you are still running into issues.

## References

<https://docs.wxwidgets.org/3.3/plat_osx_install.html>

<https://sourceforge.net/p/dvdstyler/discussion/318795/thread/d7442b04/>

<https://sourceforge.net/p/dvdstyler/discussion/318795/thread/4c443677/>
