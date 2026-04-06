# How to Build DVDStyler on MacOS

DVD Styler is an exceptionally difficult application to build, especially on MacOS. There are not great instructions out there for doing this, so I decided to compile my own.

## Notes

After the `make -j` command, enter the number of performance cores on your Mac (e.g. I have an M4 Max,
which has 12 performance and 4 efficiency cores, so I would use the number 12).

## 1 Install Prerequisites

You will need [Homebrew](https://brew.sh/) and XCode in order to follow this procedure.

## 2 Create a symlink to /opt/local

This will allow `make` to pull in external libraries from your homebrew cellar.

```bash
sudo ln -s /opt/homebrew /opt/local
```

## 3 Install Prerequisites using Homebrew

More dependencies, this time managed by homebrew.
Do not install the homebrew version of wxwidgets. We will be building it separately later in this procedure.

```bash
brew install ffmpeg dvdauthor autoconf automake libtool libconfig libexif libsvg-cairo xmlto cdrtools mjpegtools dylibbundler
```

## 4 Download and build wxSVG

git clone <https://git.code.sf.net/p/wxsvg/git> wxsvg-git
cd wxsvg-git
./autogen.sh
./configure --enable-shared
make -j12
sudo make install

## 5 Download and build wxWidgets

Download wxWidgets 3.3.1 source code (there doesn't appear to be a repository for it anywhere)

```bash
./configure --enable-shared --with-wx-config=/usr/local/bin/wx-config
make -j12
sudo make install
```

### Note

Until I figure out the linking issue, modify the line of the generated makefile and add the following to the WX_CXXFLAGS:
`-I/opt/homebrew/Cellar/fontconfig/2.17.1/include`

## 6 Build DVDStyler

Clone this repo.

```bash
./autogen.sh
./configure
make -j12
sudo make DVDStyler.app
sudo make libs
```

## References

<https://docs.wxwidgets.org/3.3/plat_osx_install.html>

<https://sourceforge.net/p/dvdstyler/discussion/318795/thread/d7442b04/>

<https://sourceforge.net/p/dvdstyler/discussion/318795/thread/4c443677/>
