///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../private/technique.h"
#include <string>
#include <vector>
#include "DX12Utils/logfn.h"
#include "DX12Utils/dxutils.h"

namespace fastnoise
{
    // Compile time technique settings. Feel free to modify these.
    static const int c_numSRVDescriptors = 256;  // If 0, no heap will be created. One heap shared by all contexts of this technique.
    static const int c_numRTVDescriptors = 256;  // If 0, no heap will be created. One heap shared by all contexts of this technique.
    static const int c_numDSVDescriptors = 256;  // If 0, no heap will be created. One heap shared by all contexts of this technique.
    static const bool c_debugShaders = true; // If true, will compile shaders with debug info enabled.
    static const bool c_debugNames = true; // If true, will set debug names on objects. If false, debug names should be deadstripped from the executable.

    // Information about the technique
    static const bool c_requiresRaytracing = false; // If true, this technique will not work without raytracing support

    using TPerfEventBeginFn = void (*)(const char* name, ID3D12GraphicsCommandList* commandList, int index);
    using TPerfEventEndFn = void (*)(ID3D12GraphicsCommandList* commandList);

    struct ProfileEntry
    {
        const char* m_label = nullptr;
        float m_gpu = 0.0f;
        float m_cpu = 0.0f;
    };

    struct Context
    {
        static const char* GetTechniqueName()
        {
            return "fastnoise";
        }

        static const wchar_t* GetTechniqueNameW()
        {
            return L"fastnoise";
        }

        // This is the input to the technique that you are expected to fill out
        struct ContextInput
        {

            // Variables
            uint3 variable_TextureSize = {{64, 64, 1}};  // The size of the output texture
            uint variable_rngSeed = 1338;  // Used during texture initialization
            uint variable_Iteration = 0;  // The current iteration
            int3 variable_filterMin = {{0,0,0}};  // Minimum range of the filter in each dimension
            int3 variable_filterMax = {{0,0,0}};  // Maximum range of the filter in each dimension
            int3 variable_filterOffset = {{0,0,0}};  // Offset into the filter buffer
            uint variable_swapSuppression = 64;
            FilterType variable_filterX = FilterType::Box;
            FilterType variable_filterY = FilterType::Box;
            FilterType variable_filterZ = FilterType::Box;
            float4 variable_filterXparams = {1,0,0,0};
            float4 variable_filterYparams = {1,0,0,0};
            float4 variable_filterZparams = {1,0,0,0};
            bool variable_separate = false;  // Whether to use "separate" mode, which makes STBN-style samples
            float variable_separateWeight = 0.500000f;  // If "separate" is true, the weight for blending between temporal and spatial filter
            SampleSpace variable_sampleSpace = SampleSpace::Real;
            SampleDistribution variable_sampleDistribution = SampleDistribution::Uniform1D;
            uint4 variable_key = {0,0,0,0};  // Used for generating random permutations
            uint variable_scrambleBits = 0;  // Number of bits to use in randomization
            bool variable_InitFromBuffer = false;

            ID3D12Resource* buffer_Filter = nullptr;
            DXGI_FORMAT buffer_Filter_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
            unsigned int buffer_Filter_stride = 0; // For structured buffers, the size of the structure
            unsigned int buffer_Filter_count = 0; // How many items there are
            D3D12_RESOURCE_STATES buffer_Filter_state = D3D12_RESOURCE_STATE_COMMON;

            static const D3D12_RESOURCE_FLAGS c_buffer_Filter_flags =  D3D12_RESOURCE_FLAG_NONE; // Flags the buffer needs to have been created with

            ID3D12Resource* buffer_InitBuffer = nullptr;
            DXGI_FORMAT buffer_InitBuffer_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
            unsigned int buffer_InitBuffer_stride = 0; // For structured buffers, the size of the structure
            unsigned int buffer_InitBuffer_count = 0; // How many items there are
            D3D12_RESOURCE_STATES buffer_InitBuffer_state = D3D12_RESOURCE_STATE_COMMON;

            static const D3D12_RESOURCE_FLAGS c_buffer_InitBuffer_flags =  D3D12_RESOURCE_FLAG_NONE; // Flags the buffer needs to have been created with
        };
        ContextInput m_input;

        // This is the output of the technique that you can consume
        struct ContextOutput
        {

            ID3D12Resource* texture_Texture = nullptr;
            unsigned int texture_Texture_size[3] = { 0, 0, 0 };
            unsigned int texture_Texture_numMips = 0;
            DXGI_FORMAT texture_Texture_format = DXGI_FORMAT_UNKNOWN;
            static const D3D12_RESOURCE_FLAGS texture_Texture_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            const D3D12_RESOURCE_STATES c_texture_Texture_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            ID3D12Resource* buffer_Data = nullptr;
            DXGI_FORMAT buffer_Data_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
            unsigned int buffer_Data_stride = 0; // For structured buffers, the size of the structure
            unsigned int buffer_Data_count = 0; // How many items there are
            const D3D12_RESOURCE_STATES c_buffer_Data_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            static const D3D12_RESOURCE_FLAGS c_buffer_Data_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Flags the buffer needs to have been created with

            ID3D12Resource* texture_SwapDebug = nullptr;
            unsigned int texture_SwapDebug_size[3] = { 0, 0, 0 };
            unsigned int texture_SwapDebug_numMips = 0;
            DXGI_FORMAT texture_SwapDebug_format = DXGI_FORMAT_UNKNOWN;
            static const D3D12_RESOURCE_FLAGS texture_SwapDebug_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            const D3D12_RESOURCE_STATES c_texture_SwapDebug_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        };
        ContextOutput m_output;

        // Internal storage for the technique
        ContextInternal m_internal;

        // If true, will do both cpu and gpu profiling. Call ReadbackProfileData() on the context to get the profiling data.
        bool m_profile = false;
        const ProfileEntry* ReadbackProfileData(ID3D12CommandQueue* commandQueue, int& numItems);

        // Set this static function pointer to your own log function if you want to recieve callbacks on info, warnings and errors.
        static TLogFn LogFn;

        // These callbacks are for perf instrumentation, such as with Pix.
        static TPerfEventBeginFn PerfEventBeginFn;
        static TPerfEventEndFn PerfEventEndFn;

        // The path to where the shader files for this technique are. Defaults to L"./"
        static std::wstring s_techniqueLocation;

        static int GetContextCount();
        static Context* GetContext(int index);

        // Buffer Creation
        template <typename T>
        ID3D12Resource* CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const T* data, size_t count, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST)
        {
            return CreateManagedBuffer(device, commandList, flags, (void*)data, count * sizeof(T), debugName, desiredState);
        }

        template <typename T>
        ID3D12Resource* CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const T& data, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST)
        {
            return CreateManagedBuffer(device, commandList, flags, (void*)&data, sizeof(T), debugName, desiredState);
        }

        template <typename T>
        ID3D12Resource* CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const std::vector<T>& data, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST)
        {
            return CreateManagedBuffer(device, commandList, flags, (void*)data.data(), data.size() * sizeof(T), debugName, desiredState);
        }

        ID3D12Resource* CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const void* data, size_t size, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST);

        // Texture Creation

        ID3D12Resource* CreateManagedTexture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, const void* initialData, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST);
        ID3D12Resource* CreateManagedTextureAndClear(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, void* clearValue, size_t clearValueSize, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST);
        ID3D12Resource* CreateManagedTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, DX12Utils::ResourceType resourceType, const char* fileName, bool sourceIsSRGB, unsigned int size[3], const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST);

        // Helpers for the host app
        void UploadTextureData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* texture, D3D12_RESOURCE_STATES textureState, const void* data, unsigned int unalignedPitch);
        void UploadBufferData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* buffer, D3D12_RESOURCE_STATES bufferState, const void* data, unsigned int dataSize);

        // The resource will be freed when the context is destroyed
        void AddManagedResource(ID3D12Resource* resource)
        {
            m_internal.m_managedResources.push_back(resource);
        }

        // Returns the allocated index within the respective heap
        int GetRTV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_RTV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName);
        int GetDSV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_DSV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName);

        bool CreateManagedTLAS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* vertexBuffer, int vertexBufferCount, bool isAABBs, D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, DXGI_FORMAT vertexPositionFormat, unsigned int vertexPositionOffset, unsigned int vertexPositionStride, ID3D12Resource*& blas, unsigned int& blasSize, ID3D12Resource*& tlas, unsigned int& tlasSize, TLogFn logFn)
        {
            ID3D12Resource* scratch = nullptr;
            ID3D12Resource* instanceDescs = nullptr;

            if (!DX12Utils::CreateTLAS(device, commandList, vertexBuffer, vertexBufferCount, isAABBs, geometryFlags, buildFlags, vertexPositionFormat, vertexPositionOffset, vertexPositionStride, blas, blasSize, tlas, tlasSize, scratch, instanceDescs, LogFn))
                return false;

            AddManagedResource(scratch);
            AddManagedResource(instanceDescs);

            AddManagedResource(blas);
            AddManagedResource(tlas);

            return true;
        }

    private:
        friend void DestroyContext(Context* context);
        ~Context();

        friend void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
        void EnsureResourcesCreated(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
        bool EnsureDrawCallPSOsCreated(ID3D12Device* device, bool dirty);

        ProfileEntry m_profileData[3+1]; // One for each action node, and another for the total
    };

    struct ScopedPerfEvent
    {
        ScopedPerfEvent(const char* name, ID3D12GraphicsCommandList* commandList, int index)
            : m_commandList(commandList)
        {
            Context::PerfEventBeginFn(name, commandList, index);
        }

        ~ScopedPerfEvent()
        {
            Context::PerfEventEndFn(m_commandList);
        }

        ID3D12GraphicsCommandList* m_commandList;
    };

    // Create 0 to N contexts at any point
    Context* CreateContext(ID3D12Device* device);

    // Call at the beginning of your frame
    void OnNewFrame(int framesInFlight);

    // Call this 0 to M times a frame on each context to execute the technique
    void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

    // Destroy a context
    void DestroyContext(Context* context);

    struct Struct_DataStruct
    {
        unsigned int initialized = false;
        uint iterationSum = 0;
        uint swaps = 0;
    };
};
