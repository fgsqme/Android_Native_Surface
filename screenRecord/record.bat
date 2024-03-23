adb push ..\outputs\exec\arm64-v8a\nativeScreenRecord /data/local/tmp/nativeScreenRecord
adb shell su -c  chmod -R +x /data/local/tmp/nativeScreenRecord
@REM adb shell su -c /data/local/tmp/NativeSurface
