///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#include "../public/technique.h"
#include "dxutils.h"

#include <vector>
#include <chrono>

namespace fastnoise
{
    static std::vector<Context*> s_allContexts;

    static DXUtils::Heap s_srvHeap;
    static DXUtils::UploadBufferTracker s_ubTracker;

    TLogFn Context::LogFn = [] (int level, const char* msg, ...) {};
    TPerfEventBeginFn Context::PerfEventBeginFn = [] (const char* name, ID3D12CommandList* commandList, int index) {};
    TPerfEventEndFn Context::PerfEventEndFn = [] (ID3D12CommandList* commandList) {};
    TLoadTextureFn Context::LoadTextureFn = [] (LoadTextureData& data) { Context::LogFn((int)LogLevel::Error, "A texture needs to be loaded but no load texture callback has been given!"); return false; };

    std::wstring Context::s_techniqueLocation = L"fastnoise/";
    static unsigned int s_timerIndex = 0;

    ID3D12CommandSignature* ContextInternal::s_commandSignatureDispatch = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Initialise_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Initialise_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_CalculateLoss_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_CalculateLoss_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Swap_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Swap_rootSig = nullptr;

    bool CreateShared(ID3D12Device* device)
    {

        // Compute Shader: Initialise
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Texture
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // Data
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 1;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // _cb
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DXUtils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_Initialise_rootSig, (c_debugNames ? L"Initialise" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO* defines = nullptr;

            if(!DXUtils::MakeComputePSO(device, Context::s_techniqueLocation.c_str(), L"shaders/init.hlsl", "Init", "cs_5_1", defines,
               ContextInternal::computeShader_Initialise_rootSig, &ContextInternal::computeShader_Initialise_pso, c_debugShaders, (c_debugNames ? L"Initialise" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: CalculateLoss
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[4];

            // LossTexture
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // Filter
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // SampleTexture
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 1;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            // _cb
            ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[3].NumDescriptors = 1;
            ranges[3].BaseShaderRegister = 0;
            ranges[3].RegisterSpace = 0;
            ranges[3].OffsetInDescriptorsFromTableStart = 3;

            if(!DXUtils::MakeRootSig(device, ranges, 4, samplers, 0, &ContextInternal::computeShader_CalculateLoss_rootSig, (c_debugNames ? L"CalculateLoss" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO* defines = nullptr;

            if(!DXUtils::MakeComputePSO(device, Context::s_techniqueLocation.c_str(), L"shaders/loss.hlsl", "Loss", "cs_5_1", defines,
               ContextInternal::computeShader_CalculateLoss_rootSig, &ContextInternal::computeShader_CalculateLoss_pso, c_debugShaders, (c_debugNames ? L"CalculateLoss" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: Swap
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[5];

            // LossTexture
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // SampleTexture
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // SwapDebug
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 1;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            // Data
            ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[3].NumDescriptors = 1;
            ranges[3].BaseShaderRegister = 2;
            ranges[3].RegisterSpace = 0;
            ranges[3].OffsetInDescriptorsFromTableStart = 3;

            // _cb
            ranges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[4].NumDescriptors = 1;
            ranges[4].BaseShaderRegister = 0;
            ranges[4].RegisterSpace = 0;
            ranges[4].OffsetInDescriptorsFromTableStart = 4;

            if(!DXUtils::MakeRootSig(device, ranges, 5, samplers, 0, &ContextInternal::computeShader_Swap_rootSig, (c_debugNames ? L"Swap" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO* defines = nullptr;

            if(!DXUtils::MakeComputePSO(device, Context::s_techniqueLocation.c_str(), L"shaders/swap.hlsl", "Swap", "cs_5_1", defines,
               ContextInternal::computeShader_Swap_rootSig, &ContextInternal::computeShader_Swap_pso, c_debugShaders, (c_debugNames ? L"Swap" : nullptr), Context::LogFn))
                return false;
        }

        // Create SRV heap
        if(c_numSRVDescriptors > 0 && !CreateHeap(s_srvHeap, device, c_numSRVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, Context::LogFn))
            return false;

        // create indirect dispatch command
        {
            D3D12_INDIRECT_ARGUMENT_DESC dispatchArg = {};
            dispatchArg.Type						 = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

            D3D12_COMMAND_SIGNATURE_DESC dispatchDesc = {};
            dispatchDesc.ByteStride					  = sizeof(uint32_t) * 3;
            dispatchDesc.NumArgumentDescs			  = 1;
            dispatchDesc.pArgumentDescs				  = &dispatchArg;
            dispatchDesc.NodeMask					  = 0x0;

            device->CreateCommandSignature(
                &dispatchDesc,
                nullptr,
                IID_PPV_ARGS(&ContextInternal::s_commandSignatureDispatch));
        }

        return true;
    }

    void DestroyShared()
    {

        if(ContextInternal::computeShader_Initialise_pso)
        {
            ContextInternal::computeShader_Initialise_pso->Release();
            ContextInternal::computeShader_Initialise_pso = nullptr;
        }

        if(ContextInternal::computeShader_Initialise_rootSig)
        {
            ContextInternal::computeShader_Initialise_rootSig->Release();
            ContextInternal::computeShader_Initialise_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_CalculateLoss_pso)
        {
            ContextInternal::computeShader_CalculateLoss_pso->Release();
            ContextInternal::computeShader_CalculateLoss_pso = nullptr;
        }

        if(ContextInternal::computeShader_CalculateLoss_rootSig)
        {
            ContextInternal::computeShader_CalculateLoss_rootSig->Release();
            ContextInternal::computeShader_CalculateLoss_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_Swap_pso)
        {
            ContextInternal::computeShader_Swap_pso->Release();
            ContextInternal::computeShader_Swap_pso = nullptr;
        }

        if(ContextInternal::computeShader_Swap_rootSig)
        {
            ContextInternal::computeShader_Swap_rootSig->Release();
            ContextInternal::computeShader_Swap_rootSig = nullptr;
        }

        // Destroy SRV Heap
        DestroyHeap(s_srvHeap);

        // Destroy any upload buffers
        s_ubTracker.Release();

        // Destroy indirect dispatch command
        if (ContextInternal::s_commandSignatureDispatch)
        {
            ContextInternal::s_commandSignatureDispatch->Release();
            ContextInternal::s_commandSignatureDispatch = nullptr;
        }
    }

    Context* CreateContext(ID3D12Device* device)
    {
        if (s_allContexts.size() == 0)
        {
            if(!CreateShared(device))
                return nullptr;
        }

        Context* ret = new Context;
        s_allContexts.push_back(ret);
        return ret;
    }

    void DestroyContext(Context* context)
    {
        s_allContexts.erase(std::remove(s_allContexts.begin(), s_allContexts.end(), context), s_allContexts.end());
        delete context;
        if (s_allContexts.size() == 0)
            DestroyShared();
    }

    void OnNewFrame(int framesInFlight)
    {
        s_ubTracker.OnNewFrame(framesInFlight);
    }

    int Context::GetContextCount()
    {
        return (int)s_allContexts.size();
    }

    Context* Context::GetContext(int index)
    {
        if (index >= 0 && index < GetContextCount())
            return s_allContexts[index];
        else
            return nullptr;
    }

    const ProfileEntry* Context::ReadbackProfileData(ID3D12CommandQueue* commandQueue, int& numItems)
    {
        numItems = 0;

        if (!m_profile || !m_internal.m_TimestampReadbackBuffer)
            return nullptr;

        uint64_t GPUFrequency;
        commandQueue->GetTimestampFrequency(&GPUFrequency);
        double GPUTickDelta = 1.0 / static_cast<double>(GPUFrequency);

        D3D12_RANGE range;
        range.Begin = 0;
        range.End = ((3 + 1) * 2) * sizeof(uint64_t);

        uint64_t* timeStampBuffer = nullptr;
        m_internal.m_TimestampReadbackBuffer->Map(0, &range, (void**)&timeStampBuffer);

        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Initialise
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: CalculateLoss
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Swap
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+1] - timeStampBuffer[0])); numItems++; // GPU total

        D3D12_RANGE emptyRange = {};
        m_internal.m_TimestampReadbackBuffer->Unmap(0, &emptyRange);

        return m_profileData;
    }

    Context::~Context()
    {
        if(m_internal.m_TimestampQueryHeap)
        {
            m_internal.m_TimestampQueryHeap->Release();
            m_internal.m_TimestampQueryHeap = nullptr;
        }
        if(m_internal.m_TimestampReadbackBuffer)
        {
            m_internal.m_TimestampReadbackBuffer->Release();
            m_internal.m_TimestampReadbackBuffer = nullptr;
        }

        if(m_output.texture_Texture)
        {
            m_output.texture_Texture->Release();
            m_output.texture_Texture = nullptr;
        }

        if(m_output.buffer_Data)
        {
            m_output.buffer_Data->Release();
            m_output.buffer_Data = nullptr;
        }

        // For storing values of the loss function
        if(m_internal.texture_Loss)
        {
            m_internal.texture_Loss->Release();
            m_internal.texture_Loss = nullptr;
        }

        if(m_output.texture_SwapDebug)
        {
            m_output.texture_SwapDebug->Release();
            m_output.texture_SwapDebug = nullptr;
        }

        // _InitCB
        if (m_internal.constantBuffer__InitCB)
        {
            m_internal.constantBuffer__InitCB->Release();
            m_internal.constantBuffer__InitCB = nullptr;
        }

        // _LossCB
        if (m_internal.constantBuffer__LossCB)
        {
            m_internal.constantBuffer__LossCB->Release();
            m_internal.constantBuffer__LossCB = nullptr;
        }

        // _SwapCB
        if (m_internal.constantBuffer__SwapCB)
        {
            m_internal.constantBuffer__SwapCB->Release();
            m_internal.constantBuffer__SwapCB = nullptr;
        }
    }

    void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        // reset the timer index
        s_timerIndex = 0;

        Context::PerfEventBeginFn("fastnoise", commandList, 11);

        std::chrono::high_resolution_clock::time_point startPointCPUTechnique;
        if(context->m_profile)
        {
            startPointCPUTechnique = std::chrono::high_resolution_clock::now();
            if(context->m_internal.m_TimestampQueryHeap == nullptr)
            {
                D3D12_QUERY_HEAP_DESC QueryHeapDesc;
                QueryHeapDesc.Count = (3+1) * 2;
                QueryHeapDesc.NodeMask = 1;
                QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                device->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&context->m_internal.m_TimestampQueryHeap));
                if (c_debugNames)
                    context->m_internal.m_TimestampQueryHeap->SetName(L"fastnoise Time Stamp Query Heap");

                context->m_internal.m_TimestampReadbackBuffer = DXUtils::CreateBuffer(device, sizeof(uint64_t) * (3+1) * 2, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (c_debugNames ? L"fastnoise Time Stamp Query Heap" : nullptr), nullptr);
            }
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
        }

        // Make sure internally owned resources are created and are the right size and format
        context->EnsureResourcesCreated(device, commandList);

        // set the SRV heap
        commandList->SetDescriptorHeaps(1, &s_srvHeap.m_heap);

        // Make sure imported textures are in the correct state
        {
            int barrierCount = 0;
            D3D12_RESOURCE_BARRIER barriers[1];

            if(context->m_input.buffer_Filter_state != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.buffer_Filter;
                barriers[barrierCount].Transition.StateBefore = context->m_input.buffer_Filter_state;
                barriers[barrierCount].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }

            if(barrierCount > 0)
                commandList->ResourceBarrier(barrierCount, barriers);
        }

        // Shader Constants: _InitCB
        {
            context->m_internal.constantBuffer__InitCB_cpu.key = context->m_input.variable_key;
            context->m_internal.constantBuffer__InitCB_cpu.Iteration = context->m_input.variable_Iteration;
            context->m_internal.constantBuffer__InitCB_cpu.scrambleBits = context->m_input.variable_scrambleBits;
            context->m_internal.constantBuffer__InitCB_cpu.rngSeed = context->m_input.variable_rngSeed;
            context->m_internal.constantBuffer__InitCB_cpu.sampleDistribution = (int)context->m_input.variable_sampleDistribution;
            DXUtils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__InitCB, context->m_internal.constantBuffer__InitCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].UAV.pResource = context->m_output.texture_Texture;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].UAV.pResource = context->m_output.buffer_Data;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: Initialise
        {
            Context::PerfEventBeginFn("Compute Shader: Initialise", commandList, 2);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Initialise_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Initialise_pso);

            DXUtils::ResourceDescriptor descriptors[] = {
                { context->m_output.texture_Texture, context->m_output.texture_Texture_format, DXUtils::AccessType::UAV, DXUtils::ResourceType::Texture3D, false, 0, context->m_output.texture_Texture_size[2] },
                { context->m_output.buffer_Data, context->m_output.buffer_Data_format, DXUtils::AccessType::UAV, DXUtils::ResourceType::Buffer, false, context->m_output.buffer_Data_stride, context->m_output.buffer_Data_count },
                { context->m_internal.constantBuffer__InitCB, DXGI_FORMAT_UNKNOWN, DXUtils::AccessType::CBV, DXUtils::ResourceType::Buffer, false, 256, 1 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_Texture_size[0],
                context->m_output.texture_Texture_size[1],
                context->m_output.texture_Texture_size[2]
            };

            unsigned int dispatchSize[3] = {
                ((baseDispatchSize[0] + 7) * 1) / 8 + 0,
                ((baseDispatchSize[1] + 7) * 1) / 8 + 0,
                ((baseDispatchSize[2] + 0) * 1) / 1 + 0
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Initialise";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
            Context::PerfEventEndFn(commandList);
        }

        // Shader Constants: _LossCB
        {
            context->m_internal.constantBuffer__LossCB_cpu.sampleSpace = (int)context->m_input.variable_sampleSpace;
            context->m_internal.constantBuffer__LossCB_cpu.separateWeight = context->m_input.variable_separateWeight;
            context->m_internal.constantBuffer__LossCB_cpu.separate = context->m_input.variable_separate;
            context->m_internal.constantBuffer__LossCB_cpu.key = context->m_input.variable_key;
            context->m_internal.constantBuffer__LossCB_cpu.scrambleBits = context->m_input.variable_scrambleBits;
            context->m_internal.constantBuffer__LossCB_cpu.TextureSize = context->m_input.variable_TextureSize;
            context->m_internal.constantBuffer__LossCB_cpu.filterMin = context->m_input.variable_filterMin;
            context->m_internal.constantBuffer__LossCB_cpu.filterMax = context->m_input.variable_filterMax;
            context->m_internal.constantBuffer__LossCB_cpu.filterOffset = context->m_input.variable_filterOffset;
            DXUtils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__LossCB, context->m_internal.constantBuffer__LossCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_output.texture_Texture;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_internal.texture_Loss;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: CalculateLoss
        {
            Context::PerfEventBeginFn("Compute Shader: CalculateLoss", commandList, 3);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_CalculateLoss_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_CalculateLoss_pso);

            DXUtils::ResourceDescriptor descriptors[] = {
                { context->m_internal.texture_Loss, context->m_internal.texture_Loss_format, DXUtils::AccessType::UAV, DXUtils::ResourceType::Texture3D, false, 0, context->m_internal.texture_Loss_size[2] },
                { context->m_input.buffer_Filter, context->m_input.buffer_Filter_format, DXUtils::AccessType::SRV, DXUtils::ResourceType::Buffer, false, context->m_input.buffer_Filter_stride, context->m_input.buffer_Filter_count },
                { context->m_output.texture_Texture, context->m_output.texture_Texture_format, DXUtils::AccessType::SRV, DXUtils::ResourceType::Texture3D, false, 0, context->m_output.texture_Texture_size[2] },
                { context->m_internal.constantBuffer__LossCB, DXGI_FORMAT_UNKNOWN, DXUtils::AccessType::CBV, DXUtils::ResourceType::Buffer, false, 256, 1 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 4, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_Texture_size[0],
                context->m_output.texture_Texture_size[1],
                context->m_output.texture_Texture_size[2]
            };

            unsigned int dispatchSize[3] = {
                ((baseDispatchSize[0] + 7) * 1) / 8 + 0,
                ((baseDispatchSize[1] + 7) * 1) / 8 + 0,
                ((baseDispatchSize[2] + 0) * 1) / 1 + 0
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "CalculateLoss";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
            Context::PerfEventEndFn(commandList);
        }

        // Shader Constants: _SwapCB
        {
            context->m_internal.constantBuffer__SwapCB_cpu.Iteration = context->m_input.variable_Iteration;
            context->m_internal.constantBuffer__SwapCB_cpu.key = context->m_input.variable_key;
            context->m_internal.constantBuffer__SwapCB_cpu.scrambleBits = context->m_input.variable_scrambleBits;
            context->m_internal.constantBuffer__SwapCB_cpu.TextureSize = context->m_input.variable_TextureSize;
            context->m_internal.constantBuffer__SwapCB_cpu.swapSuppression = context->m_input.variable_swapSuppression;
            DXUtils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__SwapCB, context->m_internal.constantBuffer__SwapCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[4];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_output.texture_Texture;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].UAV.pResource = context->m_output.buffer_Data;

            barriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[2].Transition.pResource = context->m_internal.texture_Loss;
            barriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[3].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[3].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[3].UAV.pResource = context->m_output.texture_SwapDebug;

            commandList->ResourceBarrier(4, barriers);
        }

        // Compute Shader: Swap
        {
            Context::PerfEventBeginFn("Compute Shader: Swap", commandList, 5);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Swap_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Swap_pso);

            DXUtils::ResourceDescriptor descriptors[] = {
                { context->m_internal.texture_Loss, context->m_internal.texture_Loss_format, DXUtils::AccessType::SRV, DXUtils::ResourceType::Texture3D, false, 0, context->m_internal.texture_Loss_size[2] },
                { context->m_output.texture_Texture, context->m_output.texture_Texture_format, DXUtils::AccessType::UAV, DXUtils::ResourceType::Texture3D, false, 0, context->m_output.texture_Texture_size[2] },
                { context->m_output.texture_SwapDebug, context->m_output.texture_SwapDebug_format, DXUtils::AccessType::UAV, DXUtils::ResourceType::Texture3D, false, 0, context->m_output.texture_SwapDebug_size[2] },
                { context->m_output.buffer_Data, context->m_output.buffer_Data_format, DXUtils::AccessType::UAV, DXUtils::ResourceType::Buffer, false, context->m_output.buffer_Data_stride, context->m_output.buffer_Data_count },
                { context->m_internal.constantBuffer__SwapCB, DXGI_FORMAT_UNKNOWN, DXUtils::AccessType::CBV, DXUtils::ResourceType::Buffer, false, 256, 1 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 5, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_Texture_size[0],
                context->m_output.texture_Texture_size[1],
                context->m_output.texture_Texture_size[2]
            };

            unsigned int dispatchSize[3] = {
                ((baseDispatchSize[0] + 7) * 1) / 8 + 0,
                ((baseDispatchSize[1] + 7) * 1) / 8 + 0,
                ((baseDispatchSize[2] + 0) * 1) / 1 + 0
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Swap";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
            Context::PerfEventEndFn(commandList);
        }

        // Make sure imported textures are put back in the state they were given to us in
        {
            int barrierCount = 0;
            D3D12_RESOURCE_BARRIER barriers[1];

            if(context->m_input.buffer_Filter_state != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
            {
                barriers[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barriers[barrierCount].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barriers[barrierCount].Transition.pResource = context->m_input.buffer_Filter;
                barriers[barrierCount].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                barriers[barrierCount].Transition.StateAfter = context->m_input.buffer_Filter_state;
                barriers[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrierCount++;
            }

            if(barrierCount > 0)
                commandList->ResourceBarrier(barrierCount, barriers);
        }

        if(context->m_profile)
        {
            context->m_profileData[(s_timerIndex-1)/2].m_label = "Total";
            context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPUTechnique).count();
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            commandList->ResolveQueryData(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, s_timerIndex, context->m_internal.m_TimestampReadbackBuffer, 0);
        }

        Context::PerfEventEndFn(commandList);
    }

    void Context::EnsureResourcesCreated(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {

        // Texture
        {

            unsigned int baseSize[3] = { (unsigned int)m_input.variable_TextureSize[0], (unsigned int)m_input.variable_TextureSize[1], (unsigned int)m_input.variable_TextureSize[2] };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

            if(!m_output.texture_Texture ||
               m_output.texture_Texture_size[0] != desiredSize[0] ||
               m_output.texture_Texture_size[1] != desiredSize[1] ||
               m_output.texture_Texture_size[2] != desiredSize[2] ||
               m_output.texture_Texture_format != desiredFormat)
            {
                if(m_output.texture_Texture)
                    m_output.texture_Texture->Release();

                m_output.texture_Texture = DXUtils::CreateTexture(device, desiredSize, desiredFormat, m_output.texture_Texture_flags, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DXUtils::ResourceType::Texture3D, (c_debugNames ? L"Texture" : nullptr), Context::LogFn);
                m_output.texture_Texture_size[0] = desiredSize[0];
                m_output.texture_Texture_size[1] = desiredSize[1];
                m_output.texture_Texture_size[2] = desiredSize[2];
                m_output.texture_Texture_format = desiredFormat;
            }
        }

        // Data
        {
            unsigned int baseCount = 1;
            unsigned int desiredCount = ((baseCount + 0 ) * 1) / 1 + 0;
            DXGI_FORMAT desiredFormat = DXGI_FORMAT_UNKNOWN;
            unsigned int desiredStride = 12;

            if(!m_output.buffer_Data ||
               m_output.buffer_Data_count != desiredCount ||
               m_output.buffer_Data_format != desiredFormat ||
               m_output.buffer_Data_stride != desiredStride)
            {
                if(m_output.buffer_Data)
                    m_output.buffer_Data->Release();

                unsigned int desiredSize = desiredCount * ((desiredStride > 0) ? desiredStride : DXUtils::SizeOfFormat(desiredFormat, Context::LogFn));

                m_output.buffer_Data = DXUtils::CreateBuffer(device, desiredSize, m_output.c_buffer_Data_flags, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"Data" : nullptr), Context::LogFn);
                m_output.buffer_Data_count = desiredCount;
                m_output.buffer_Data_format = desiredFormat;
                m_output.buffer_Data_stride = desiredStride;
            }
        }

        // Loss
        // For storing values of the loss function
        {

            unsigned int baseSize[3] = { (unsigned int)m_input.variable_TextureSize[0], (unsigned int)m_input.variable_TextureSize[1], (unsigned int)m_input.variable_TextureSize[2] };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R32_FLOAT;

            if(!m_internal.texture_Loss ||
               m_internal.texture_Loss_size[0] != desiredSize[0] ||
               m_internal.texture_Loss_size[1] != desiredSize[1] ||
               m_internal.texture_Loss_size[2] != desiredSize[2] ||
               m_internal.texture_Loss_format != desiredFormat)
            {
                if(m_internal.texture_Loss)
                    m_internal.texture_Loss->Release();

                m_internal.texture_Loss = DXUtils::CreateTexture(device, desiredSize, desiredFormat, m_internal.texture_Loss_flags, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, DXUtils::ResourceType::Texture3D, (c_debugNames ? L"Loss" : nullptr), Context::LogFn);
                m_internal.texture_Loss_size[0] = desiredSize[0];
                m_internal.texture_Loss_size[1] = desiredSize[1];
                m_internal.texture_Loss_size[2] = desiredSize[2];
                m_internal.texture_Loss_format = desiredFormat;
            }
        }

        // SwapDebug
        {

            unsigned int baseSize[3] = { (unsigned int)m_input.variable_TextureSize[0], (unsigned int)m_input.variable_TextureSize[1], (unsigned int)m_input.variable_TextureSize[2] };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            DXGI_FORMAT desiredFormat = m_output.texture_Texture_format;

            if(!m_output.texture_SwapDebug ||
               m_output.texture_SwapDebug_size[0] != desiredSize[0] ||
               m_output.texture_SwapDebug_size[1] != desiredSize[1] ||
               m_output.texture_SwapDebug_size[2] != desiredSize[2] ||
               m_output.texture_SwapDebug_format != desiredFormat)
            {
                if(m_output.texture_SwapDebug)
                    m_output.texture_SwapDebug->Release();

                m_output.texture_SwapDebug = DXUtils::CreateTexture(device, desiredSize, desiredFormat, m_output.texture_SwapDebug_flags, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DXUtils::ResourceType::Texture3D, (c_debugNames ? L"SwapDebug" : nullptr), Context::LogFn);
                m_output.texture_SwapDebug_size[0] = desiredSize[0];
                m_output.texture_SwapDebug_size[1] = desiredSize[1];
                m_output.texture_SwapDebug_size[2] = desiredSize[2];
                m_output.texture_SwapDebug_format = desiredFormat;
            }
        }

        // _InitCB
        if (m_internal.constantBuffer__InitCB == nullptr)
            m_internal.constantBuffer__InitCB = DXUtils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_InitCB" : nullptr), Context::LogFn);

        // _LossCB
        if (m_internal.constantBuffer__LossCB == nullptr)
            m_internal.constantBuffer__LossCB = DXUtils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_LossCB" : nullptr), Context::LogFn);

        // _SwapCB
        if (m_internal.constantBuffer__SwapCB == nullptr)
            m_internal.constantBuffer__SwapCB = DXUtils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_SwapCB" : nullptr), Context::LogFn);
    }
};
