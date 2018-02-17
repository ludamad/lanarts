#!/usr/bin/env sh

ndk-build NDK_MODULE_PATH="${ROOT}"
gradlew assembleDebug
adb install -r build/outputs/apk/proj.android-debug.apk
adb shell am start -n org.oxygine.${PROJECT}/org.oxygine.${PROJECT}.MainActivity