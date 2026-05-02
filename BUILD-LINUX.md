# How to Build DVDStyler on Linux

Tested on Ubuntu LTS 24.04. Your mileage may vary.

## Notes

## 1 Install Prerequisites

```bash
sudo apt install libtool flex libwxgtk3.2-dev libwxgtk3.2-dev libwxgtk-media3.2-1t64 pkg-config libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libavfilter-dev libwxsvg-dev libcairo-dev libpango1.0-dev wx3.2-headers libexif-dev ffmpeg gettext dvdauthor xmlto
```

## 2 Build DVDStyler

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
