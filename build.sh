#!/bin/bash
cmake -G Ninja -S ./ -B build/  \
  -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK?}/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="arm64-v8a" -DANDROID_PLATFORM=android-30
cmake --build build/