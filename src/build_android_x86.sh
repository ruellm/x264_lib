#!/bin/bash
NDK=$ANDROID_NDK  #your ndk root path
PLATFORM=$NDK/platforms/android-14/arch-x86
PREBUILT=$NDK/toolchains/x86-4.8/prebuilt/linux-x86_64

function build_one()
{
./configure --host=i686-linux \
--prefix=. \
--enable-shared \
--enable-static \
--enable-pic \
--disable-asm \
--extra-ldflags=-lm \
--disable-gpl \
--disable-swscale \
--disable-lavf \
--disable-ffms \
--disable-gpac \
--disable-cli \
--cross-prefix=$PREBUILT/bin/i686-linux-android- \
--sysroot=$PLATFORM \
--extra-cflags='-fno-aggressive-loop-optimizations'

}
build_one
