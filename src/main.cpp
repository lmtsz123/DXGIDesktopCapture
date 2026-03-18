#include "DXGICapture.h"
#include "Mp4Recorder.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

int main() {
    const HRESULT comHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(comHr)) {
        std::wcerr << L"Failed to initialize COM." << std::endl;
        return -1;
    }

    std::wcout << L"DXGI Desktop Capture Demo" << std::endl;
    std::wcout << L"Press ESC to exit" << std::endl;

    DXGICapture capture;
    
    if (!capture.Initialize()) {
        std::wcerr << L"Failed to initialize DXGI capture" << std::endl;
        CoUninitialize();
        return -1;
    }

    std::wcout << L"Screen Size: " << capture.GetWidth() 
               << L"x" << capture.GetHeight() << std::endl;
    std::wcout << L"Capture started..." << std::endl;

    Mp4Recorder recorder;
    const UINT32 targetFps = 30;
    if (!recorder.Initialize(L"capture.mp4",
                             static_cast<UINT32>(capture.GetWidth()),
                             static_cast<UINT32>(capture.GetHeight()),
                             targetFps)) {
        std::wcerr << L"Failed to initialize MP4 recorder." << std::endl;
        CoUninitialize();
        return -1;
    }

    std::vector<uint8_t> frameBuffer;

    int frameCount = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    while (true) {
        if (capture.CaptureFrame()) {
            if (capture.HasNewFrame()) {
                if (capture.CopyCapturedFrameToBuffer(frameBuffer)) {
                    if (recorder.WriteFrameBGRA(frameBuffer.data(), frameBuffer.size())) {
                        frameCount++;
                    }
                }

                auto currentTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>
                               (currentTime - startTime);

                if (duration.count() >= 1) {
                    std::wcout << L"Recorded FPS: " << frameCount << std::endl;
                    frameCount = 0;
                    startTime = currentTime;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
    }

    recorder.Finalize();
    std::wcout << L"Capture stopped." << std::endl;
    CoUninitialize();
    return 0;
}