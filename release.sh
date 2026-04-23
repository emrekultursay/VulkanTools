#!/bin/bash

# TODO: do NOT submit this. it's a convenience script to copy .so files, build
# the APK and then copy the APK.

set -e
set -x


cp build-android/install/arm64-v8a/lib/libVkLayer_CPUTiming.so ~/studio-main/tools/profiler/native/android/sherlocklayersapp/app/src/main/lib/arm64-v8a/libVkLayer_CPUTiming.so
cp build-android/install/arm64-v8a/lib/libVkLayer_DebugMarker.so ~/studio-main/tools/profiler/native/android/sherlocklayersapp/app/src/main/lib/arm64-v8a/libVkLayer_DebugMarker.so
cp build-android/install/arm64-v8a/lib/libVkLayer_api_dump.so ~/studio-main/tools/profiler/native/android/sherlocklayersapp/app/src/main/lib/arm64-v8a/libVkLayer_api_dump.so


pushd ~/studio-main/tools/profiler/

./scripts/build/build_layers_apk.py
cp native/android/sherlocklayersapp/app/build/outputs/apk/debug/sherlock-layers.apk ~/studio-main/prebuilts/tools/sherlock/sherlock-apk/sherlock-layers.apk


echo "Done!"
