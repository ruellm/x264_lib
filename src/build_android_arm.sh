#!/bin/bash

API_LEVEL=23
TARGET_HOST=arm-linux-androideabi
PREBUILT=$ANDROID_NDK/standalones/arm_${API_LEVEL}_libc++_clang

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

CC=$PREBUILT/bin/${TARGET_HOST}-clang \
./configure \
    --host=${TARGET_HOST} \
    --cross-prefix=$PREBUILT/bin/${TARGET_HOST}- \
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
