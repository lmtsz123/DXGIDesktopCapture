#pragma once

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <cstdint>

class Mp4Recorder {
public:
    Mp4Recorder();
    ~Mp4Recorder();

    bool Initialize(const std::wstring& outputPath, UINT32 width, UINT32 height, UINT32 fps);
    bool WriteFrameBGRA(const uint8_t* bgraData, size_t dataSize);
    void Finalize();

    bool IsInitialized() const { return m_initialized; }

private:
    bool EnsureMediaFoundationStarted();
    void LogHr(const wchar_t* context, HRESULT hr) const;

    Microsoft::WRL::ComPtr<IMFSinkWriter> m_sinkWriter;
    DWORD m_streamIndex;
    LONGLONG m_rtStart;
    LONGLONG m_frameDuration;
    UINT32 m_width;
    UINT32 m_height;
    UINT32 m_inputStride;
    bool m_initialized;
    bool m_mfStarted;
};
