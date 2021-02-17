#!/usr/bin/env bash

API_LEVEL=16
ANDROID_ABI=arm
ANDROID_HOST=linux-androideabi

while [[ $# -gt 0 ]]
then
    if [[ $1 == "-a" ]] || [[ $1 == "--arch" ]]
    then
        shift 1
        ANDROID_ABI="$1"
    if [[ $1 == "--api" ]]
    then
        shift 1
        API_LEVEL="$1"
    fi

    shift 1
fi

ANDROID_ABI_SHORT=arm

TARGET_HOST=${ANDROID_ABI}-${ANDROID_HOST}
PREBUILT=$ANDROID_NDK/standalones/${ANDROID_ABI_SHORT}_${API_LEVEL}_libc++_clang

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
