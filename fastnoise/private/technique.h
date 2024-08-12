///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace fastnoise
{
    using uint = unsigned int;
    using uint2 = std::array<uint, 2>;
    using uint3 = std::array<uint, 3>;
    using uint4 = std::array<uint, 4>;

    using int2 = std::array<int, 2>;
    using int3 = std::array<int, 3>;
    using int4 = std::array<int, 4>;
    using float2 = std::array<float, 2>;
    using float3 = std::array<float, 3>;
    using float4 = std::array<float, 4>;
    using float4x4 = std::array<std::array<float, 4>, 4>;

    enum class FilterType: int
    {
        Box,
        Gaussian,
        Binomial,
        Exponential,
        WeightedExponential,
    };

    enum class SampleSpace: int
    {
        Real,
        Circle,
        Vector2,
        Vector3,
        Vector4,
        Sphere,
    };

    enum class SampleDistribution: int
    {
        Uniform1D,
        Gauss1D,
        Tent1D,
        Uniform2D,
        Uniform3D,
        Uniform4D,
        UniformSphere,
        UniformHemisphere,
        CosineHemisphere,
    };

    inline const char* EnumToString(FilterType value, bool displayString = false)
    {
        switch(value)
        {
            case FilterType::Box: return displayString ? "Box" : "Box";
            case FilterType::Gaussian: return displayString ? "Gaussian" : "Gaussian";
            case FilterType::Binomial: return displayString ? "Binomial" : "Binomial";
            case FilterType::Exponential: return displayString ? "Exponential" : "Exponential";
            case FilterType::WeightedExponential: return displayString ? "WeightedExponential" : "WeightedExponential";
            default: return nullptr;
        }
    }

    inline const char* EnumToString(SampleSpace value, bool displayString = false)
    {
        switch(value)
        {
            case SampleSpace::Real: return displayString ? "Real" : "Real";
            case SampleSpace::Circle: return displayString ? "Circle" : "Circle";
            case SampleSpace::Vector2: return displayString ? "Vector2" : "Vector2";
            case SampleSpace::Vector3: return displayString ? "Vector3" : "Vector3";
            case SampleSpace::Vector4: return displayString ? "Vector4" : "Vector4";
            case SampleSpace::Sphere: return displayString ? "Sphere" : "Sphere";
            default: return nullptr;
        }
    }

    inline const char* EnumToString(SampleDistribution value, bool displayString = false)
    {
        switch(value)
        {
            case SampleDistribution::Uniform1D: return displayString ? "Uniform1D" : "Uniform1D";
            case SampleDistribution::Gauss1D: return displayString ? "Gauss1D" : "Gauss1D";
            case SampleDistribution::Tent1D: return displayString ? "Tent1D" : "Tent1D";
            case SampleDistribution::Uniform2D: return displayString ? "Uniform2D" : "Uniform2D";
            case SampleDistribution::Uniform3D: return displayString ? "Uniform3D" : "Uniform3D";
            case SampleDistribution::Uniform4D: return displayString ? "Uniform4D" : "Uniform4D";
            case SampleDistribution::UniformSphere: return displayString ? "UniformSphere" : "UniformSphere";
            case SampleDistribution::UniformHemisphere: return displayString ? "UniformHemisphere" : "UniformHemisphere";
            case SampleDistribution::CosineHemisphere: return displayString ? "CosineHemisphere" : "CosineHemisphere";
            default: return nullptr;
        }
    }

    struct ContextInternal
    {
        ID3D12QueryHeap* m_TimestampQueryHeap = nullptr;
        ID3D12Resource* m_TimestampReadbackBuffer = nullptr;

        static ID3D12CommandSignature* s_commandSignatureDispatch;

        struct Struct__InitCB
        {
            unsigned int InitFromBuffer = false;
            uint Iteration = 0;  // The current iteration
            float2 _padding0 = {};  // Padding
            uint4 key = {0,0,0,0};  // Used for generating random permutations
            uint rngSeed = 1338;  // Used during texture initialization
            int sampleDistribution = (int)SampleDistribution::Uniform1D;
            uint scrambleBits = 0;  // Number of bits to use in randomization
            float _padding1 = 0.000000f;  // Padding
        };

        struct Struct__LossCB
        {
            uint3 TextureSize = {{64, 64, 1}};  // The size of the output texture
            float _padding0 = 0.000000f;  // Padding
            int3 filterMax = {{0,0,0}};  // Maximum range of the filter in each dimension
            float _padding1 = 0.000000f;  // Padding
            int3 filterMin = {{0,0,0}};  // Minimum range of the filter in each dimension
            float _padding2 = 0.000000f;  // Padding
            int3 filterOffset = {{0,0,0}};  // Offset into the filter buffer
            float _padding3 = 0.000000f;  // Padding
            uint4 key = {0,0,0,0};  // Used for generating random permutations
            int sampleSpace = (int)SampleSpace::Real;
            uint scrambleBits = 0;  // Number of bits to use in randomization
            unsigned int separate = false;  // Whether to use "separate" mode, which makes STBN-style samples
            float separateWeight = 0.500000f;  // If "separate" is true, the weight for blending between temporal and spatial filter
        };

        struct Struct__SwapCB
        {
            uint Iteration = 0;  // The current iteration
            uint3 TextureSize = {{64, 64, 1}};  // The size of the output texture
            uint4 key = {0,0,0,0};  // Used for generating random permutations
            uint scrambleBits = 0;  // Number of bits to use in randomization
            uint swapSuppression = 64;
            float2 _padding0 = {};  // Padding
        };

        // For storing values of the loss function
        ID3D12Resource* texture_Loss = nullptr;
        unsigned int texture_Loss_size[3] = { 0, 0, 0 };
        unsigned int texture_Loss_numMips = 0;
        DXGI_FORMAT texture_Loss_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture_Loss_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        const D3D12_RESOURCE_STATES c_texture_Loss_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        Struct__InitCB constantBuffer__InitCB_cpu;
        ID3D12Resource* constantBuffer__InitCB = nullptr;

        static ID3D12PipelineState* computeShader_Initialise_pso;
        static ID3D12RootSignature* computeShader_Initialise_rootSig;

        Struct__LossCB constantBuffer__LossCB_cpu;
        ID3D12Resource* constantBuffer__LossCB = nullptr;

        static ID3D12PipelineState* computeShader_CalculateLoss_pso;
        static ID3D12RootSignature* computeShader_CalculateLoss_rootSig;

        Struct__SwapCB constantBuffer__SwapCB_cpu;
        ID3D12Resource* constantBuffer__SwapCB = nullptr;

        static ID3D12PipelineState* computeShader_Swap_pso;
        static ID3D12RootSignature* computeShader_Swap_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
