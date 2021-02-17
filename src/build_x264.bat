@echo off
echo "Building x264"
if "%~1" == "mvs12" call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
if "%~1" == "mvs15" call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
if "%~1" == "" 	    call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86

@rem set path for msys
set PATH=C:\MinGW\msys\1.0\bin;%PATH% 

echo "Current work directory " %CD%
echo "Clean work directory..."
bash -c "make clean"
echo "Configure project"
bash -c "CC=cl ./configure --disable-cli --enable-shared --prefix=${PWD}/installed"
echo "Make project"
bash -c "make"
echo "Install project"
bash -c "make install"
echo "Done!"