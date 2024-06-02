## Downloading Qt

Qt source bundles can be downloaded from (https://download.qt.io/archive/qt/).

5.15.14: (https://download.qt.io/archive/qt/5.15/5.15.14/single/qt-everywhere-opensource-src-5.15.14.zip)
6.6.3: (https://download.qt.io/archive/qt/6.6/6.6.3/single/qt-everywhere-src-6.6.3.zip)

## MacOS

Building on macos is very straightforward. The only prerequisite you will need is Xcode, the official binary is built with Xcode 13.2.1 but any recent version should work.

- First, extract the Qt source to its own directory.
- In that directory, run the command `configure -release -opensource -opengl desktop -prefix ../qt-6.6.3`
- Then run `cmake --build . --parallel` and `cmake --install .`

## Windows

Building Qt on windows is a little weird and I always have trouble finding the official documentation for it so here is a quick summary.

### Prerequisites

- Visual Studio 2022 for Qt 6
- Visual Studio 2019 for Qt 5
- A recent version of Perl, I recommend [Strawberry Perl](https://strawberryperl.com/)
- A recent version of Python 3

### File Paths

This document will assume you have a top-level directory `C:\Qt`. Source code is extracted to a version-specific subdirectory named like `C:\Qt\src-6.6.3`. Binaries will be installed into a directory named like `C:\Qt\6.6.3_vs2022` based on the Qt and Visual Studio version.

### Setting up a command prompt

To configure and build Qt, you will need to use a Visual Studio command prompt that also has some additional Qt directories on its PATH. You can create such a command prompt by first making a file `C:\Qt\setup-6.6.3.cmd` with the following contents, substituting version numbers where appropriate.

For Qt 5 use Visual Studio 2019 and for Qt 6 use Visual Studio 2022.

```
CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
SET _ROOT=C:\Qt\src-6.6.3
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%
SET _ROOT=
```

With this file created, create a shortcut with the following parameters:

Target: `%SystemRoot%\system32\cmd.exe /E:ON /V:ON /k C:\Qt\setup-6.6.3.cmd`
Start in: `C:\Qt\src-6.6.3`

This shortcut should now open a command prompt in your source directory ready to run configure.

Note that the most recent version of Visual Studio supported by Qt 5 is VS 2019.

### Configure, Build, and Install

The build system has changed in Qt 6 so this step is different depending on your version. Run the following commands to build and install both debug and release libraries.

#### Qt 6
- `configure -debug-and-release -opensource -opengl desktop -no-openssl -skip qtsensors -prefix ../6.6.3_vs2022`
- `cmake --build . --parallel`
- `cmake --install . --config Debug`
- `cmake --install .`

#### Qt 5
- `configure -debug-and-release -opensource -opengl desktop -ssl -schannel -no-openssl -prefix ../5.15.14_vs2019`
- `nmake`
- `nmake install`
