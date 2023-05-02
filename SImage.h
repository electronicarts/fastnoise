///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DX12.h"
#include <vector>

struct SImage
{
    ~SImage();

    enum PixelConversions
    {
        PixelsAreU8_SaveAsU8,
        PixelsAreF32_SaveAsU8,
        PixelsAreF32_SaveAsF32
    };

    bool Load(ID3D12Device* device, const char* fileName);
    bool Save(const char* fileName, PixelConversions pixelConversion);
    bool SaveRegion(const char* fileName, int x1, int x2, int y1, int y2, PixelConversions pixelConversion);

    void AdoptResource(ID3D12Resource* resource, int width, int height, int components, DXGI_FORMAT format, int bytesPerComponent)
    {
        if (m_resource && m_releaseResource)
            m_resource->Release();

        m_releaseResource = false;

        m_width = width;
        m_height = height;
        m_components = components;
        m_format = format;

        m_pixels.resize(width * height * components * bytesPerComponent);

        m_resource = resource;
    }

    void UploadPixelsToGPU(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void RequestReadback(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void DoReadback();

    // CPU data
    int m_width = 0;
    int m_height = 0;
    int m_components = 0;
    std::vector<unsigned char> m_pixels;
    std::vector<unsigned char> m_F23ToU8pixels;

    // GPU data
    bool m_releaseResource = true;
    ID3D12Resource* m_resource = nullptr;
    DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;  
    std::vector<ID3D12Resource*> m_uploadBuffers;
    ID3D12Resource* m_readbackBuffer = nullptr;
    UINT64 m_readbackBufferRowBytes = 0;
    UINT64 m_readbackBufferRowPitch = 0;

    std::wstring m_debugName;
};
