call ndk-build NDK_MODULE_PATH="${ROOT}"
call gradlew assembleDebug
call adb install -r build/outputs/apk/proj.android-debug.apk
call adb shell am start -n org.oxygine.${PROJECT}/org.oxygine.${PROJECT}.MainActivity