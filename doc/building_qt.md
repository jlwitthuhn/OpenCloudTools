# Building Qt

Building Qt on windows is a little weird and I always have trouble finding the official documentation for it so here is a quick summary.

## Prerequisites

- Visual Studio 2022 for Qt 6
- Visual Studio 2019 for Qt 5
- A recent version of Perl, I recommend [Strawberry Perl](https://strawberryperl.com/)
- A recent version of Python 3

## Getting Qt

Qt source bundles can be downloaded from (https://download.qt.io/archive/qt/).

5.15.9: (https://download.qt.io/archive/qt/5.15/5.15.9/single/qt-everywhere-opensource-src-5.15.9.tar.xz)
6.5.1: (https://download.qt.io/archive/qt/6.5/6.5.1/single/qt-everywhere-src-6.5.1.tar.xz)

## File Paths

This document will assume you have a top-level directory `C:\Qt`. Source code is extracted to a version-specific subdirectory named like `C:\Qt\6.5.1_src`. Binaries will be installed into a directory named like `C:\Qt\6.5.1_vs2022` based on the Qt and Visual Studio version.

## Setting up a command prompt

To configure and build Qt, you will need to use a Visual Studio command prompt that also has some additional Qt directories on its PATH. You can create such a command prompt by first making a file `C:\Qt\6.5.1_setup.cmd` with the following contents, substituting version numbers where appropriate.

```
CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
SET _ROOT=C:\Qt\6.5.1_src
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%
SET _ROOT=
```

With this file created, create a shortcut with the following parameters:

Target: `%SystemRoot%\system32\cmd.exe /E:ON /V:ON /k C:\Qt\6.5.1_setup.cmd`
Start in: `C:\Qt\6.5.1_src`

This shortcut should now open a command prompt in your source directory ready to run configure.

Note that the most recent version of Visual Studio supported by Qt 5 is VS 2019.

## Configure, Build, and Install

The build system has changed in Qt 6 so this step is different depending on your version. Run the following commands to build and install both debug and release libraries.

### Qt 6
- `configure -debug-and-release -opensource -opengl desktop -no-openssl -skip qtsensors -prefix ../6.5.1_vs2022`
- `cmake --build . --config Debug`
- `cmake --install . --config Debug`
- `cmake --build .`
- `cmake --install .`

### Qt 5
- `configure -debug-and-release -opensource -opengl desktop -ssl -schannel -no-openssl -prefix ../5.15.9_vs2019`
- `nmake`
- `nmake install`
