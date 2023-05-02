///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>
#include <vector>
#include <unordered_map>
#include "shadercompiler.h"

#define ALIGN(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

namespace fastnoise
{
enum class LogLevel : int;
using TLogFn = void (*)(int level, const char* msg, ...);

namespace DXUtils
{
    enum class AccessType
    {
        SRV,
        UAV,
        CBV,
    };

    enum class ResourceType
    {
        Buffer,
        Texture2D,
        Texture2DArray,
        Texture3D,
        RTScene
    };

    struct Heap
    {
        ID3D12DescriptorHeap* m_heap = nullptr;
        size_t indexCount = 0;
        size_t nextIndexFree = 0;
        size_t indexSize = 0;

        std::unordered_map<size_t, size_t> descriptorTableCache;
    };

    struct UploadBufferTracker
    {
        struct Buffer
        {
            ID3D12Resource* buffer = nullptr;
            size_t size = 0;
            size_t age = 0;
        };

        void OnNewFrame(int framesInFlight)
        {
            // advance the age of each in use buffer. Put them in the free list when it's safe to do so
            inUse.erase(
                std::remove_if(inUse.begin(), inUse.end(),
                    [framesInFlight, this] (Buffer* buffer)
                    {
                        buffer->age++;
                        if(buffer->age >= framesInFlight)
                        {
                            buffer->age = 0;
                            free.push_back(buffer);
                            return true;
                        }
                        return false;
                    }
                ),
                inUse.end()
            );
        }

        void Release()
        {
            for (Buffer* b : inUse)
                b->buffer->Release();
            inUse.clear();

            for (Buffer* b : free)
                b->buffer->Release();
            free.clear();
        }

        Buffer* GetBuffer(ID3D12Device* device, size_t size, fastnoise::TLogFn logFn);

        std::vector<Buffer*> inUse;
        std::vector<Buffer*> free;
    };

    bool CreateHeap(Heap& heap, ID3D12Device* device, int numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, fastnoise::TLogFn logFn);
    void DestroyHeap(Heap& heap);

    ID3D12Resource* CreateTexture(ID3D12Device* device, unsigned int size[3], DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, ResourceType textureType, LPCWSTR debugName, fastnoise::TLogFn logFn);
    ID3D12Resource* CreateBuffer(ID3D12Device* device, unsigned int size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, D3D12_HEAP_TYPE heapType, LPCWSTR debugName, fastnoise::TLogFn logFn);

    bool CopyConstantsCPUToGPU(UploadBufferTracker& tracker, ID3D12Device* device, ID3D12GraphicsCommandList* commandList,ID3D12Resource* resource, void* data, size_t dataSize, fastnoise::TLogFn logFn);

    template <typename T>
    bool CopyConstantsCPUToGPU(UploadBufferTracker& tracker, ID3D12Device* device, ID3D12GraphicsCommandList* commandList,ID3D12Resource* resource, const T&data, fastnoise::TLogFn logFn)
    {
        return CopyConstantsCPUToGPU(tracker, device, commandList, resource, (void*)&data, sizeof(data), logFn);
    }

    bool MakeRootSig(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_RANGE* ranges,
        int rangeCount,
        D3D12_STATIC_SAMPLER_DESC* samplers,
        int samplerCount,
        ID3D12RootSignature** rootSig,
        LPCWSTR debugName,
        fastnoise::TLogFn logFn);

    struct ResourceDescriptor
    {
        ID3D12Resource* m_res;
        DXGI_FORMAT m_format;
        AccessType m_access;
        ResourceType m_resourceType;
        bool m_raw;

        // used by buffers, constant buffers, texture2darrays and texture3ds
        UINT m_stride;

        // used by buffers
        UINT m_count;
    };

    D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorTable(ID3D12Device* device, Heap& srvHeap, const ResourceDescriptor* descriptors, size_t count, fastnoise::TLogFn logFn);

    unsigned int SizeOfFormat(DXGI_FORMAT format, fastnoise::TLogFn logFn);
    unsigned int FormatChannelCount(DXGI_FORMAT format, fastnoise::TLogFn logFn);

    enum class FormatChannelType
    {
        U8,
        F32,

        Count
    };
    FormatChannelType GetFormatChannelType(DXGI_FORMAT format, fastnoise::TLogFn logFn);
}
};
