cmake_minimum_required(VERSION 3.11.4)

include(ExternalProject)

if (ANDROID)
    ExternalProject_Add(
        x264
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ./build_android.sh --arch ${ANDROID_ABI} --api ${ANDROID_API}
        BUILD_COMMAND make
        INSTALL_COMMAND ./install.sh ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ABI}
        BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ABI}/libx264.a
    )
elseif (WIN32)
    ExternalProject_Add(
        x264
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    ExternalProject_Add(
        x264
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND bash -l -c "./configure --prefix=. --enable-shared --enable-static --extra-cflags='-fno-aggressive-loop-optimizations'"
        BUILD_COMMAND bash -l -c "make"
        INSTALL_COMMAND bash -l -c "./install.sh ${CMAKE_CURRENT_BINARY_DIR}"
    )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    ExternalProject_Add(
        x264
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND bash -l -c "./configure --prefix=. --enable-shared --enable-static"
        BUILD_COMMAND bash -l -c "make"
        INSTALL_COMMAND bash -l -c "install_name_tool -id @rpath/libx264.155.dylib libx264.155.dylib"
            COMMAND bash -l -c "./install.sh ${CMAKE_CURRENT_BINARY_DIR}"
    )
else()
    ExternalProject_Add(
        x264
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND bash -l -c "./configure --prefix=. --libdir=. --enable-shared --enable-static"
        BUILD_COMMAND bash -l -c "make"
        INSTALL_COMMAND bash -l -c "./install.sh ${CMAKE_CURRENT_BINARY_DIR}"
    )
endif()
