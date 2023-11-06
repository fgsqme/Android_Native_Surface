.\hex\a
cmake --build cmake-build-debug --target all -- -j 16
adb -s 192.168.31.128:5555 push outputs\arm64-v8a\ /data/local/tmp/
adb -s 192.168.31.128:5555 shell su -c chmod -R +x /data/local/tmp/arm64-v8a/*

@REM adb -s 192.168.31.108:5555 push outputs\arm64-v8a\ /data/local/tmp/
@REM adb -s 192.168.31.108:5555 shell su -c chmod -R +x /data/local/tmp/arm64-v8a/*

@REM adb -s 192.168.31.228:5555 push outputs\arm64-v8a\ /data/local/tmp/
@REM adb -s 192.168.31.228:5555 shell su -c chmod -R +x /data/local/tmp/arm64-v8a/*