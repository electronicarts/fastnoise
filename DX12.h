///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <string>
#include <codecvt>
#include <locale>
#include <stdarg.h>
#include <comdef.h>
#include <chrono>
#include <atlbase.h>
#include <atlconv.h>

#define ENABLE_GPU_BASED_VALIDATION() false

#define Assert(X, MSG, ...) if ((X) == false) ShowErrorMessage( __FUNCTION__ "():\n\nExpression:\n" #X "\n\n" MSG, __VA_ARGS__);

inline void ShowErrorMessage(const char* msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    vsprintf_s(buffer, msg, args);
    va_end(args);

    MessageBoxA(nullptr, buffer, "FastNoise", MB_OK);
    DebugBreak();
    exit(100);
}

inline void ThrowIfFailed_(HRESULT hr, const char* functionName)
{
    if (FAILED(hr))
    {
        _com_error err(hr);

        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;

        std::wstring errorStringW = err.ErrorMessage();
        std::string errorString = converter.to_bytes(errorStringW);

        Assert(false, "DirectX Error in %s(): %s", functionName, errorString.c_str());
    }
}

#define ThrowIfFailed(hr) ThrowIfFailed_(hr, __FUNCTION__)

struct DX12
{
    DX12(bool useWarpDevice = false)
    {
        m_startTime = std::chrono::high_resolution_clock::now();

        if (ENABLE_GPU_BASED_VALIDATION())
        {
            ID3D12Debug* spDebugController0 = nullptr;
            ID3D12Debug1* spDebugController1 = nullptr;
            D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0));
            spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1));
            spDebugController1->SetEnableGPUBasedValidation(true);
            spDebugController0->Release();
            spDebugController1->Release();
        }

        UINT dxgiFactoryFlags = 0;

    #if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        {
            ID3D12Debug* debugController = nullptr;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
            debugController->Release();
        }
    #endif

        IDXGIFactory4* factory = nullptr;
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        if (useWarpDevice)
        {
            IDXGIAdapter* warpAdapter = nullptr;
            ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

            ThrowIfFailed(D3D12CreateDevice(
                warpAdapter,
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&m_device)
            ));

            warpAdapter->Release();
        }
        else
        {
            IDXGIAdapter1* hardwareAdapter = nullptr;
            GetHardwareAdapter(factory, &hardwareAdapter);

            ThrowIfFailed(D3D12CreateDevice(
                hardwareAdapter,
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&m_device)
            ));

            hardwareAdapter->Release();
        }

        // set it up to break on dx errors in debug
        #ifdef _DEBUG
        {
            ID3D12InfoQueue* infoQueue = nullptr;
            if (SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
            {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                D3D12_MESSAGE_ID hide[] =
                {
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
                };
                D3D12_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                infoQueue->AddStorageFilterEntries(&filter);
                infoQueue->Release();
            }
        }
        #endif

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

        // Create a command allocator
        ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

        factory->Release();

        // Create the command list.
        ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList)));

        // Command lists are created in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        ThrowIfFailed(m_commandList->Close());

        // Create synchronization objects and wait until assets have been uploaded to the GPU.
        {
            ThrowIfFailed(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
            m_fenceValue++;

            // Create an event handle to use for frame synchronization.
            m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_fenceEvent == nullptr)
            {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
    }

    ~DX12()
    {
        // wait for all work to be done before we start releasing things
        WaitForGpu();

        CloseHandle(m_fenceEvent);
        m_fence->Release();

        m_commandAllocator->Release();

        m_commandList->Release();
        m_commandQueue->Release();

        m_device->Release();

#if defined(_DEBUG)
        {
            IDXGIDebug1* dxgiDebug = nullptr;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
            {
                dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
            }
            dxgiDebug->Release();
        }
#endif

        std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
        float duration = std::chrono::duration_cast<std::chrono::duration<float>>(endTime - m_startTime).count();
        printf("Total Program Execution took %s\n", MakeDurationString(duration).c_str());
    }

    std::chrono::high_resolution_clock::time_point m_startTime;

    ID3D12Device* m_device = nullptr;

    ID3D12CommandAllocator* m_commandAllocator;
    ID3D12CommandQueue* m_commandQueue = nullptr;
    ID3D12GraphicsCommandList* m_commandList = nullptr;

    HANDLE m_fenceEvent;
    ID3D12Fence* m_fence = nullptr;
    UINT64 m_fenceValue;

    // Wait for pending GPU work to complete.
    void WaitForGpu()
    {
        // Schedule a Signal command in the queue.
        ThrowIfFailed(m_commandQueue->Signal(m_fence, m_fenceValue));

        // Wait until the fence has been processed.
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

        // Increment the fence value for the current frame.
        m_fenceValue++;
    }

    template <typename EXECUTEFN>
    void Execute(const EXECUTEFN& ExecuteFn)
    {
        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        ThrowIfFailed(m_commandAllocator->Reset());
        ThrowIfFailed(m_commandList->Reset(m_commandAllocator, nullptr));

        ExecuteFn(m_device, m_commandList);

        ThrowIfFailed(m_commandList->Close());

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = { m_commandList };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        WaitForGpu();
    }

    void GetHardwareAdapter(
        IDXGIFactory1* pFactory,
        IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false)
    {
        *ppAdapter = nullptr;

        IDXGIAdapter1* adapter = nullptr;

        IDXGIFactory6* factory6 = nullptr;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                    continue;

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                    break;
            }
        }

        if (adapter == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                    continue;

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                    break;
            }
        }

        *ppAdapter = adapter;
        factory6->Release();
    }

    std::string MakeDurationString(float durationInSeconds)
    {
        std::string ret;

        static const float c_oneMinute = 60.0f;
        static const float c_oneHour = c_oneMinute * 60.0f;

        int hours = int(durationInSeconds / c_oneHour);
        durationInSeconds -= float(hours) * c_oneHour;

        int minutes = int(durationInSeconds / c_oneMinute);
        durationInSeconds -= float(minutes) * c_oneMinute;

        int seconds = int(durationInSeconds);

        char buffer[1024];
        if (hours < 10)
            sprintf_s(buffer, "0%i:", hours);
        else
            sprintf_s(buffer, "%i:", hours);
        ret = buffer;

        if (minutes < 10)
            sprintf_s(buffer, "0%i:", minutes);
        else
            sprintf_s(buffer, "%i:", minutes);
        ret += buffer;

        if (seconds < 10)
            sprintf_s(buffer, "0%i", seconds);
        else
            sprintf_s(buffer, "%i", seconds);
        ret += buffer;

        return ret;
    }
};
