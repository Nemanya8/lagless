#pragma once
#include <d3d11.h>
#include <dxgi1_2.h>

class CaptureManager {
public:
    bool Initialize();
    ID3D11Texture2D* CaptureFrame();
    ID3D11Device* GetDevice() const { return m_device; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGIOutputDuplication* m_duplication = nullptr;
    int m_width = 1920;
    int m_height = 1080;
};
