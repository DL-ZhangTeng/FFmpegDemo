# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
#在Gradle Console输出信息
#CMAKE_SOURCE_DIR: E:/MyApplication2/app
message("CURRENT_DIR:" ${CMAKE_SOURCE_DIR})
include_directories(
        ${CMAKE_SOURCE_DIR}/src/main/cpp/include #h文件目录
        ${CMAKE_SOURCE_DIR}/libs/include
)
#支持-std=gnu++11
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        ttt
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        src/main/cpp/ttt.cpp)
add_library(
        avcodec
        SHARED
        IMPORTED
)
add_library(
        avfilter
        SHARED
        IMPORTED
)
add_library(
        avformat
        SHARED
        IMPORTED
)
add_library(
        avutil
        SHARED
        IMPORTED
)
add_library(
        swresample
        SHARED
        IMPORTED
)
add_library(
        swscale
        SHARED
        IMPORTED
)
add_library(
        fdk-aac
        SHARED
        IMPORTED
)
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavcodec.so
)
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavfilter.so
)
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavformat.so
)
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavutil.so
)
set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswresample.so
)
set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswscale.so
)
set_target_properties(
        fdk-aac
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libfdk-aac.so
)
# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ttt
        fdk-aac
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})