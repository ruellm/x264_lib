#!/bin/bash
PREBUILT=$ANDROID_NDK/standalones/arm_23_libc++_clang

if [[ ! -d $ANDROID_NDK ]]
then
    echo "No ANDROID_NDK at $ANDROID_NDK"
    exit -1
fi

if [[ ! -d $PREBUILT ]]
then
    echo "No PREBUILT at $PREBUILT"
    exit -1
fi

CC=$PREBUILT/bin/arm-linux-androideabi-clang \
./configure \
    --host=arm-linux-androideabi \
    --cross-prefix=$PREBUILT/bin/arm-linux-androideabi- \
    --sysroot=$PREBUILT/sysroot \
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
