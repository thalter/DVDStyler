# How to Build DVDStyler on Linux

Tested on Ubuntu LTS 24.04. Your mileage may vary.

## 1 Install Prerequisites

```bash
sudo apt install libtool flex libwxgtk3.2-dev libwxgtk3.2-dev libwxgtk-media3.2-dev libwxgtk-media3.2-1t64 pkg-config libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libavfilter-dev libwxsvg-dev libcairo-dev libpango1.0-dev wx3.2-headers libexif-dev ffmpeg gettext genisoimage dvd+rw-tools dvdauthor xmlto bison
```

## 1.1 Optional Components

If want support for GMOME thunbnail generation, device detection, and shutdown, install the library below

```bash
sudo apt install libgnome-desktop-3-dev
```

## 2 Build DVDStyler

Clone this repo and execute the following.

```bash
./autogen.sh
./configure
make -j$(sysctl -n hw.ncpu)
```

For a local install, execute the following:

```bash
sudo make install
```

To create a deployable artifact, execute the following:

```bash
./build_release.sh
```
