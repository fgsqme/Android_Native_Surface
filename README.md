## Android_Native_Surface

## 关于

[此项目基于 Android_Native_Surface](https://github.com/SsageParuders/Android_Native_Surface)

### 功能
- c++ surface 绘制，支持 OpenGL和Vulkan，支持跳过录屏，已修复触摸穿透问题
- c++ 录屏 / 录屏共享
- 项目支持到 Android 9 ~ 14
### 目录
- [libsurface](libsurface) c++ 绘制库
- [examples](examples) 绘制库使用示例
- [screenRecord](screenRecord) 录屏代码示例
- [outputs](outputs) 编译后文件输出路径
- [outputs/include](outputs/include)  c++ 绘制库头文件，供外部调用使用

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

### 执行方法演示

>   ```bash
>     chmod +x nativeSurface
>     ./nativeSurface
>   ```

