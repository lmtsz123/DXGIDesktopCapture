#include "DXGICapture.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::wcout << L"DXGI Desktop Capture Demo" << std::endl;
    std::wcout << L"Press ESC to exit" << std::endl;

    DXGICapture capture;
    
    if (!capture.Initialize()) {
        std::wcerr << L"Failed to initialize DXGI capture" << std::endl;
        return -1;
    }

    std::wcout << L"Screen Size: " << capture.GetWidth() 
               << L"x" << capture.GetHeight() << std::endl;
    std::wcout << L"Capture started..." << std::endl;

    int frameCount = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    while (true) {
        if (capture.CaptureFrame()) {
            frameCount++;
            
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>
                           (currentTime - startTime);
            
            if (duration.count() >= 1) {
                std::wcout << L"FPS: " << frameCount << std::endl;
                frameCount = 0;
                startTime = currentTime;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
    }

    std::wcout << L"Capture stopped." << std::endl;
    return 0;
}