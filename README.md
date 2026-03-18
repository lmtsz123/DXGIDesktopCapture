# DXGI Desktop Capture Demo

这是一个完整的Windows桌面捕获demo，使用DXGI API实现高性能的桌面录制功能。

## 功能特性

- 使用DXGI Desktop Duplication API进行高效桌面捕获
- 完整的鼠标捕获和渲染支持
- 支持所有类型的鼠标指针：
  - DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME (单色)
  - DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR (彩色)
  - DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR (带掩码彩色)
- D3D11渲染管线
- CMake构建系统

## 系统要求

- Windows 8/10/11
- Visual Studio 2019/2022 或 MinGW-w64
- CMake 3.16+
- DirectX 11支持的显卡

## 编译说明

1. 克隆项目：
```bash
git clone <your-repo-url>
cd DXGIDesktopCapture
```

2. 生成并编译（Visual Studio）：
```bash
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

3. 运行：
```bash
build/bin/Release/DXGIDesktopCapture.exe
```

程序运行后会在当前工作目录生成 `capture.mp4`。

## MP4 说明

- 当前已实现 MP4 录制输出（H.264 in MP4），使用 Windows Media Foundation 编码与封装。
- 已预留 `mp4v2` 源码引入入口：`third_party/mp4v2`。
- 如果你需要启用 `mp4v2`，将其源码放到 `third_party/mp4v2`（包含 `CMakeLists.txt`）即可被 CMake 自动检测并尝试链接。