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