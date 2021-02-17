cmake_minimum_required(VERSION 3.17)
project(x264_lib)

set(CMAKE_CXX_STANDARD 14)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-DSYS_WINDOWS)
endif()

add_library(x264_lib STATIC
        src/common/osdep.c
        src/common/base.c
        src/common/cpu.c
        src/common/tables.c
        src/encoder/api.c

        src/common/mc.c
        src/common/predict.c
        src/common/pixel.c
        src/common/macroblock.c
        src/common/frame.c
        src/common/dct.c
        src/common/cabac.c
        src/common/common.c
        src/common/rectangle.c
        src/common/set.c
        src/common/quant.c
        src/common/deblock.c
        src/common/vlc.c
        src/common/mvpred.c
        src/common/bitstream.c
        src/encoder/analyse.c

        src/encoder/me.c
        src/encoder/ratecontrol.c
        src/encoder/set.c
        src/encoder/macroblock.c
        src/encoder/cabac.c
        src/encoder/cavlc.c
        src/encoder/encoder.c
        src/encoder/lookahead.c

        #opencl
        src/common/opencl/bidir.cl
        src/common/opencl/downscale.cl
        src/common/opencl/intra.cl
        src/common/opencl/motionsearch.cl
        src/common/opencl/subpel.cl
        src/common/opencl/weightp.cl
        src/common/opencl.c
        src/encoder/slicetype-cl.c
        h264lib_x264/h264lib.h h264lib_x264/h264_encoder.cpp types.h

        h264lib_x264/h264_encoder.cpp
        adapter/x264_adapter.h adapter/x264_adapter.cpp)

target_include_directories(x264_lib PUBLIC
        src
        src/common
        src/common/encoder
        common/opencl
        h264lib_x264
        adapter
        )
