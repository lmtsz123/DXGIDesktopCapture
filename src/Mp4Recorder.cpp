#include "Mp4Recorder.h"

#include <comdef.h>
#include <iostream>
#include <cstring>

using namespace Microsoft::WRL;

Mp4Recorder::Mp4Recorder()
    : m_streamIndex(0),
      m_rtStart(0),
      m_frameDuration(0),
      m_width(0),
      m_height(0),
      m_inputStride(0),
      m_initialized(false),
      m_mfStarted(false) {}

Mp4Recorder::~Mp4Recorder() {
    Finalize();
}

bool Mp4Recorder::EnsureMediaFoundationStarted() {
    if (m_mfStarted) {
        return true;
    }

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        LogHr(L"MFStartup failed.", hr);
        return false;
    }

    m_mfStarted = true;
    return true;
}

bool Mp4Recorder::Initialize(const std::wstring& outputPath, UINT32 width, UINT32 height, UINT32 fps) {
    if (m_initialized) {
        return true;
    }

    if (!EnsureMediaFoundationStarted()) {
        return false;
    }

    if (fps == 0 || width == 0 || height == 0) {
        std::wcerr << L"Mp4Recorder::Initialize invalid video settings." << std::endl;
        return false;
    }

    m_width = width;
    m_height = height;
    m_inputStride = width * 4;
    m_frameDuration = 10000000LL / static_cast<LONGLONG>(fps);
    m_rtStart = 0;

    ComPtr<IMFAttributes> attrs;
    HRESULT hr = MFCreateAttributes(&attrs, 2);
    if (FAILED(hr)) {
        LogHr(L"MFCreateAttributes failed.", hr);
        return false;
    }

    hr = attrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
    if (FAILED(hr)) {
        LogHr(L"Set hardware transforms attribute failed.", hr);
        return false;
    }

    hr = MFCreateSinkWriterFromURL(outputPath.c_str(), nullptr, attrs.Get(), &m_sinkWriter);
    if (FAILED(hr)) {
        LogHr(L"MFCreateSinkWriterFromURL failed.", hr);
        return false;
    }

    ComPtr<IMFMediaType> outType;
    hr = MFCreateMediaType(&outType);
    if (FAILED(hr)) {
        LogHr(L"MFCreateMediaType(output) failed.", hr);
        return false;
    }

    hr = outType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr)) return false;
    hr = outType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    if (FAILED(hr)) return false;
    hr = outType->SetUINT32(MF_MT_AVG_BITRATE, width * height * 6);
    if (FAILED(hr)) return false;
    hr = outType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (FAILED(hr)) return false;
    hr = MFSetAttributeSize(outType.Get(), MF_MT_FRAME_SIZE, width, height);
    if (FAILED(hr)) return false;
    hr = MFSetAttributeRatio(outType.Get(), MF_MT_FRAME_RATE, fps, 1);
    if (FAILED(hr)) return false;
    hr = MFSetAttributeRatio(outType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    if (FAILED(hr)) return false;

    hr = m_sinkWriter->AddStream(outType.Get(), &m_streamIndex);
    if (FAILED(hr)) {
        LogHr(L"SinkWriter AddStream failed.", hr);
        return false;
    }

    ComPtr<IMFMediaType> inType;
    hr = MFCreateMediaType(&inType);
    if (FAILED(hr)) {
        LogHr(L"MFCreateMediaType(input) failed.", hr);
        return false;
    }

    hr = inType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr)) return false;
    hr = inType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    if (FAILED(hr)) return false;
    hr = inType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (FAILED(hr)) return false;
    hr = MFSetAttributeSize(inType.Get(), MF_MT_FRAME_SIZE, width, height);
    if (FAILED(hr)) return false;
    hr = MFSetAttributeRatio(inType.Get(), MF_MT_FRAME_RATE, fps, 1);
    if (FAILED(hr)) return false;
    hr = MFSetAttributeRatio(inType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    if (FAILED(hr)) return false;
    hr = inType->SetUINT32(MF_MT_DEFAULT_STRIDE, m_inputStride);
    if (FAILED(hr)) return false;

    hr = m_sinkWriter->SetInputMediaType(m_streamIndex, inType.Get(), nullptr);
    if (FAILED(hr)) {
        LogHr(L"SinkWriter SetInputMediaType failed.", hr);
        return false;
    }

    hr = m_sinkWriter->BeginWriting();
    if (FAILED(hr)) {
        LogHr(L"SinkWriter BeginWriting failed.", hr);
        return false;
    }

    m_initialized = true;
    std::wcout << L"Recording MP4 to: " << outputPath << std::endl;
    return true;
}

bool Mp4Recorder::WriteFrameBGRA(const uint8_t* bgraData, size_t dataSize) {
    if (!m_initialized || !bgraData) {
        return false;
    }

    const DWORD expectedSize = static_cast<DWORD>(m_inputStride * m_height);
    if (dataSize < expectedSize) {
        std::wcerr << L"WriteFrameBGRA input buffer too small." << std::endl;
        return false;
    }

    ComPtr<IMFMediaBuffer> buffer;
    HRESULT hr = MFCreateMemoryBuffer(expectedSize, &buffer);
    if (FAILED(hr)) {
        LogHr(L"MFCreateMemoryBuffer failed.", hr);
        return false;
    }

    BYTE* dst = nullptr;
    hr = buffer->Lock(&dst, nullptr, nullptr);
    if (FAILED(hr)) {
        LogHr(L"IMFMediaBuffer::Lock failed.", hr);
        return false;
    }

    memcpy(dst, bgraData, expectedSize);

    hr = buffer->Unlock();
    if (FAILED(hr)) {
        LogHr(L"IMFMediaBuffer::Unlock failed.", hr);
        return false;
    }

    hr = buffer->SetCurrentLength(expectedSize);
    if (FAILED(hr)) {
        LogHr(L"SetCurrentLength failed.", hr);
        return false;
    }

    ComPtr<IMFSample> sample;
    hr = MFCreateSample(&sample);
    if (FAILED(hr)) {
        LogHr(L"MFCreateSample failed.", hr);
        return false;
    }

    hr = sample->AddBuffer(buffer.Get());
    if (FAILED(hr)) {
        LogHr(L"IMFSample::AddBuffer failed.", hr);
        return false;
    }

    hr = sample->SetSampleTime(m_rtStart);
    if (FAILED(hr)) return false;
    hr = sample->SetSampleDuration(m_frameDuration);
    if (FAILED(hr)) return false;

    hr = m_sinkWriter->WriteSample(m_streamIndex, sample.Get());
    if (FAILED(hr)) {
        LogHr(L"SinkWriter WriteSample failed.", hr);
        return false;
    }

    m_rtStart += m_frameDuration;
    return true;
}

void Mp4Recorder::Finalize() {
    if (m_initialized && m_sinkWriter) {
        const HRESULT hr = m_sinkWriter->Finalize();
        if (FAILED(hr)) {
            LogHr(L"SinkWriter Finalize failed.", hr);
        }
    }

    m_sinkWriter.Reset();
    m_initialized = false;
    m_streamIndex = 0;
    m_rtStart = 0;

    if (m_mfStarted) {
        MFShutdown();
        m_mfStarted = false;
    }
}

void Mp4Recorder::LogHr(const wchar_t* context, HRESULT hr) const {
    _com_error err(hr);
    std::wcerr << context
               << L" HRESULT=0x" << std::hex << static_cast<unsigned int>(hr) << std::dec
               << L", message=" << err.ErrorMessage()
               << std::endl;
}
