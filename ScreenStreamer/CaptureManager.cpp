#include "CaptureManager.h"
#include <iostream>

bool CaptureManager::Initialize() {
    // Create D3D11 device
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &m_device,
        &featureLevel,
        &m_context
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device" << std::endl;
        return false;
    }

    // Get DXGI device
    IDXGIDevice* dxgiDevice = nullptr;
    hr = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI device" << std::endl;
        return false;
    }

    // Get DXGI adapter
    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiDevice->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI adapter" << std::endl;
        return false;
    }

    // Get primary output
    IDXGIOutput* dxgiOutput = nullptr;
    hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput);
    dxgiAdapter->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get primary output" << std::endl;
        return false;
    }

    // Get output1 interface
    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&dxgiOutput1);
    dxgiOutput->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI output1" << std::endl;
        return false;
    }

    // Create desktop duplication
    hr = dxgiOutput1->DuplicateOutput(m_device, &m_duplication);
    dxgiOutput1->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to create desktop duplication" << std::endl;
        return false;
    }

    // Get output description
    DXGI_OUTPUT_DESC outputDesc;
    dxgiOutput->GetDesc(&outputDesc);
    m_width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    m_height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

    return true;
}

ID3D11Texture2D* CaptureManager::CaptureFrame() {
    if (!m_duplication) return nullptr;

    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr = m_duplication->AcquireNextFrame(500, &frameInfo, &desktopResource);
    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_ACCESS_LOST) {
            // Handle access lost
            m_duplication->Release();
            m_duplication = nullptr;
            Initialize();
        }
        return nullptr;
    }

    // Get the desktop texture
    ID3D11Texture2D* desktopTexture = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopTexture);
    desktopResource->Release();
    if (FAILED(hr)) {
        m_duplication->ReleaseFrame();
        return nullptr;
    }

    // Create a staging texture
    D3D11_TEXTURE2D_DESC desc;
    desktopTexture->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;

    ID3D11Texture2D* stagingTexture = nullptr;
    hr = m_device->CreateTexture2D(&desc, nullptr, &stagingTexture);
    if (FAILED(hr)) {
        desktopTexture->Release();
        m_duplication->ReleaseFrame();
        return nullptr;
    }

    // Copy the desktop texture to staging texture
    m_context->CopyResource(stagingTexture, desktopTexture);
    desktopTexture->Release();
    m_duplication->ReleaseFrame();

    return stagingTexture;
}
