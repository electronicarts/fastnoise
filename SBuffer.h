///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DX12.h"
#include <vector>

template <typename T>
struct SBuffer
{
    ~SBuffer()
    {
        if (m_releaseResource && m_resource)
            m_resource->Release();
        for (ID3D12Resource* resource : m_uploadBuffers)
            resource->Release();
        if (m_readbackBuffer)
            m_readbackBuffer->Release();
    }

    bool Load(ID3D12Device* device, T* data, size_t count, const char* debugName)
    {
        m_debugName = std::wstring(CA2W(std::string(debugName).c_str()));

        // Copy the data
        m_data.resize(count);
        memcpy(m_data.data(), data, sizeof(T) * count);

        // Create a resource
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.Width = sizeof(T) * count;
        bufferDesc.Height = 1;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.SampleDesc.Quality = 0;
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 1;
        heapProperties.VisibleNodeMask = 1;

        HRESULT hr = device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            nullptr,
            IID_PPV_ARGS(&m_resource));

        // set the name for debugging purposes
        m_resource->SetName(m_debugName.c_str());

        return true;
    }

    template <size_t N>
    bool Load(ID3D12Device* device, T(&data)[N], const char* debugName)
    {
        return Load(device, data, N, debugName);
    }

    bool Load(ID3D12Device* device, T &data, const char* debugName)
    {
        return Load(device, &data, 1, debugName);
    }

    void UploadDataToGPU(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
    {
        UINT64 size = 0;
        D3D12_RESOURCE_DESC desc = m_resource->GetDesc();
        device->GetCopyableFootprints(&desc, 0, 1, 0, nullptr, nullptr, nullptr, &size);

        // create an upload heap resource
        ID3D12Resource* uploadResource = nullptr;
        {
            D3D12_HEAP_PROPERTIES heapDesc = {};
            heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
            heapDesc.CreationNodeMask = 1;
            heapDesc.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC resourceDesc = {};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Alignment = 0;
            resourceDesc.Width = size;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            HRESULT hr = device->CreateCommittedResource(
                &heapDesc,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&uploadResource));
            m_uploadBuffers.push_back(uploadResource);

            // set the name for debugging purposes
            uploadResource->SetName((m_debugName + L" Upload").c_str());
        }

        // map the upload heap resource and copy the data into it
        {
            unsigned char* gpuBufferData = nullptr;
            ThrowIfFailed(uploadResource->Map(0, nullptr, reinterpret_cast<void**>(&gpuBufferData)));
            memcpy(gpuBufferData, m_data.data(), sizeof(T) * m_data.size());
            uploadResource->Unmap(0, nullptr);
        }

        // Transition resource from unordered access to copy dest
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = m_resource;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            cmdList->ResourceBarrier(1, &barrier);
        }

        // copy the buffer data
        cmdList->CopyResource(m_resource, uploadResource);

        // Transition resource from copy dest to unordered access
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = m_resource;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            cmdList->ResourceBarrier(1, &barrier);
        }
    }

    void RequestReadback(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ID3D12Resource* sourceBuffer = nullptr, D3D12_RESOURCE_STATES sourceBufferState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
    {
        if (!sourceBuffer)
            sourceBuffer = m_resource;

        UINT64 size = 0;
        D3D12_RESOURCE_DESC desc = m_resource->GetDesc();
        device->GetCopyableFootprints(&desc, 0, 1, 0, nullptr, nullptr, nullptr, &size);

        // create a readback heap resource if it doesn't yet exist
        if (!m_readbackBuffer)
        {
            D3D12_HEAP_PROPERTIES heapDesc = {};
            heapDesc.Type = D3D12_HEAP_TYPE_READBACK;
            heapDesc.CreationNodeMask = 1;
            heapDesc.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC resourceDesc = {};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Alignment = 0;
            resourceDesc.Width = size;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            HRESULT hr = device->CreateCommittedResource(
                &heapDesc,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_readbackBuffer));

            // set the name for debugging purposes
            m_readbackBuffer->SetName((m_debugName + L" Readback").c_str());
        }

        // Transition resource from unordered access to copy source
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = sourceBuffer;
            barrier.Transition.StateBefore = sourceBufferState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            cmdList->ResourceBarrier(1, &barrier);
        }

        // copy the buffer data
        cmdList->CopyResource(m_readbackBuffer, sourceBuffer);

        // Transition resource from copy source to unordered access
        {
            D3D12_RESOURCE_BARRIER barrier;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = sourceBuffer;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barrier.Transition.StateAfter = sourceBufferState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            cmdList->ResourceBarrier(1, &barrier);
        }
    }

    void DoReadback()
    {
        unsigned char* gpuBufferData = nullptr;
        ThrowIfFailed(m_readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&gpuBufferData)));
        memcpy(m_data.data(), gpuBufferData, sizeof(T) * m_data.size());
        m_readbackBuffer->Unmap(0, nullptr);
    }

    void AdoptResource(ID3D12Resource* resource, size_t count)
    {
        if (m_resource && m_releaseResource)
            m_resource->Release();

        m_releaseResource = false;

        m_data.resize(count);

        m_resource = resource;
    }

    // CPU data
    std::vector<T> m_data;

    // GPU data
    bool m_releaseResource = true;
    ID3D12Resource* m_resource = nullptr;
    std::vector<ID3D12Resource*> m_uploadBuffers;
    ID3D12Resource* m_readbackBuffer = nullptr;

    std::wstring m_debugName;
};
