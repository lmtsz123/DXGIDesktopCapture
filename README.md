# DXGIDesktopCapture

Windows 桌面采集 Demo：用 **DXGI Desktop Duplication** 抓主屏，把鼠标叠到画面上，再经 **Media Foundation** 写成 H.264 MP4。

按 `ESC` 结束录制，当前工作目录会生成 `capture.mp4`。

## 功能

- DXGI Desktop Duplication 采集主显示器（`EnumOutputs(0)`）
- 鼠标指针叠加：单色 / 彩色 / Masked Color
- D3D11 纹理管线，staging 回读为 BGRA
- Media Foundation SinkWriter 输出 H.264 MP4（目标 30 fps）
- 会话丢失时自动恢复（`DXGI_ERROR_ACCESS_LOST`，例如分辨率变化、锁屏、UAC）

## 环境

- Windows 8 / 10 / 11（需要桌面会话，远程/服务会话通常不行）
- DirectX 11 显卡
- CMake 3.16+
- C++17 编译器：Visual Studio 2019/2022，或 MinGW-w64
- 链接：`d3d11` `dxgi` `d3dcompiler` `mfplat` `mfreadwrite` `mfuuid` `ole32`

## 编译

仓库根目录的 CMake 文件目前叫 `CmakeLists.txt`（Windows 上大小写不敏感，可以直接用）。

**Visual Studio 2022：**

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

**Visual Studio 2019：** 把生成器换成 `"Visual Studio 16 2019"`。

**Ninja / MinGW：**

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

可执行文件在 `build/bin/Release/DXGIDesktopCapture.exe`（VS 多配置）或 `build/bin/DXGIDesktopCapture.exe`（单配置）。

## 运行

```bash
build/bin/Release/DXGIDesktopCapture.exe
```

- 控制台会打印分辨率和大约每秒写入的帧数
- 输出文件：运行时工作目录下的 `capture.mp4`
- `ESC` 停止并 finalize 文件

建议在资源管理器里双击运行，或 `cd` 到想保存视频的目录再启动，避免文件写到构建目录里。

## 结构

```
src/main.cpp            入口：COM 初始化、采集循环、ESC 退出
src/DXGICapture.*       D3D11 设备、桌面复制、ACCESS_LOST 恢复、BGRA 回读
src/MouseHandler.*      指针形状更新，绘制到采集纹理
src/Mp4Recorder.*       MF SinkWriter：RGB32 → H.264 MP4
third_party/mp4v2/      可选 mp4v2 源码占位，默认未接入编码器
CmakeLists.txt          工程文件
```

数据流：`AcquireNextFrame` → 复制桌面纹理 → 画鼠标 → staging 回读 BGRA → `Mp4Recorder::WriteFrameBGRA`。

## mp4v2（可选，目前未真正使用）

CMake 有 `ENABLE_MP4V2`（默认 ON），但 **当前编码器走的是 Media Foundation**。`Mp4Recorder` 没有调用 mp4v2。只有把带 `CMakeLists.txt` 的 mp4v2 源码放到 `third_party/mp4v2` 时才会尝试链接，例如：

```bash
git clone https://github.com/TechSmith/mp4v2.git third_party/mp4v2
```

没有这份源码时 CMake 只会警告，不影响 MF 录制。

关掉探测：

```bash
cmake -S . -B build -DENABLE_MP4V2=OFF
```

## 限制

- 只采主屏，没有音频
- 帧经过 CPU 回读，不是 GPU 直出编码
- 目标 30 fps，循环里还 sleep 了约 16 ms，实际帧率取决于桌面更新和编码
- 没有预览窗口
- `SaveTextureToFile` 仍是空实现

## 许可

仓库尚未添加 LICENSE。
