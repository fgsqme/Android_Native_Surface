## Android_Native_Surface

## 关于

[此项目基于 Android_Native_Surface](https://github.com/SsageParuders/Android_Native_Surface)

### 功能

- Android c++ 绘制和录屏，目前只测试了安卓12和12.1，触摸穿透只在安卓13设备测试过，安卓14卡触摸问题在x86_64设备上测试无问题，其他版本需要自行测试

### libs

- 项目支持到 Android 9 ~ 14

---

### 效果图

- 录屏
  <br> <br>
  <img width="300" alt="image" src="gif/record.png">
- 绘制
  <br> <br>
  <img width="300" alt="image" src="gif/imgui.png">

---

### aosp_res/ android level /aosp_native_surface

- 编译
    - 下载aosp项目，复制对应aosp_res下的安卓版本到aosp根目录的 external/aosp_native_surface
    - cd到external/aosp_native_surface执行mm命令编译
    - 编译完成后输出路径在out/target/product/system/(lib,lib64)/libSsage.so

---

### 此项目编译输出路径 outputs

> 执行方法演示 <br>
>   ```bash
>     chmod +x NativeSurface
>     ./NativeSurface
>   ```

