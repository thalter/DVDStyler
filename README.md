# DVDStyler

This is a fork of the [DVDStyler SourceForge git Repo](https://sourceforge.net/p/dvdstyler/DVDStyler/ci/master/tree/). DVDStyler does not seem to be well maintained anymore, and is difficult to build (especially on MacOS).

The MacOS port is very neglected. The last official binary (3.2.1) is very old - among other issues, it does not
support high-DPI displays and is not ARM native (it is an Intel build that runs under Rosetta2). This means that DVDStyler will no longer work under MacOS 27, as x64 support will be removed. 
Building from source is the only way to get support for the latest Apple hardware and OS versions, at least for now.

This fork is based on the unreleased 3.3b4 code base, which, as near as I can tell, is fully functional.

## Build Instructions

See [BUILD.md](BUILD.md) for the latest MacOS build instructions (as well as build instructions for Windows and Linux)

## Reference

You can also view the original [README](README) file.
