///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#define NOMINMAX
#include "DX12.h"
#include "SImage.h"
#include "SBuffer.h"
#include <random>
#include <string>

#include "fastnoise/public/technique.h"

#define DETERMINISTIC() false
#define OUTPUT_INTERMEDIATE_IMAGES() false

std::string g_outputFileName;
bool g_outputLayersAsSingleImages = false;
size_t g_numSteps = 10000;

static void LogFn(int level, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
}

void PrintUsage()
{
    printf(
        "\n"
        "FastNoise.exe <sampleSpace> <distribution> <filterXY> <filterZ> <filterCombine> <textureSize> <fileName> [-split]\n"
        "\n"
        "  <sampleSpace>  - The type of value stored in each pixel.\n"
        "                     Real | Circle | Vector2 | Vector3 | Vector4 | Sphere\n"
        "\n"
        "  <distribution> - The distribution of values.\n"
        "                     Uniform | Tent | Cosine | UniformHemisphere | Gauss\n"
        "\n"
        "  <filterXY>     - Spatial filter and parameters.\n"
        "                     Box <Size> |\n"
        "                     Gauss <Sigma> |\n"
        "                     Binomial <N>\n"
        "\n"
        "  <filterZ>      - Temporal filter and parameters.\n"
        "                     Box <Size> |\n"
        "                     Gauss <Sigma> |\n"
        "                     Binomial <N> | \n"
        "                     Exponential <Alpha> <Beta>\n"
        "\n"
        "  <filterCombine> - How to combine the spatial and temporal filters.\n"
        "                     Separate <spatialWeight> |\n"
        "                     Product |\n"
        "\n"
        "  <textureSize>   - The dimensions of the texture: x y z.\n"
        "\n"
        "  <fileName>      - The path and filename to output, without a file extension.\n"
        "\n"
        "  -split          - If specified, each slice will be output as a separate image, else, all\n"
        "                    slices will be put together into a single image.\n"
        "\n"
        "  -numsteps <steps> - specify how many iterations of optimization to do. defualts to 10,000.\n"
        "\n"
        "\n"
        "Parameter Explanation:\n"
        "- Box size is diameter, so 3 gives you 3x3, 5 gives you 5x5 etc.\n"
        "- Binomial N is the N in N choose K, so 2 gives 3x3, 4 gives 5x5 etc.\n"
        "- Exponential filter alpha is the value used in the lerp. See our paper for the meaning of beta.\n"
        "- spatialWeight is how much to weigh the spatial filter. 1-spatialWeight is given to the temporal.\n"
        "\n"
        "\n"
        "Note: Using gauss distribution will cause the output file to be a .hdr file to contain floating point\n"
        "values. The values will be between 0 and 1, since .hdr files can't store negative numbers. To convert\n"
        "these values to a sigma=1.0 gaussian distribution, subtract 0.5, then divide by 0.15.\n"
        "\n"
        "\n"
        "Example:\n"
        "  FastNoise.exe real uniform gauss 1.0 exponential 0.2 0.2 separate 0.5 128 128 64 out -split\n"
        "\n"
        "The above command generates 64 slices of noise which are each 128x128.  The noise is optimized\n"
        "for a Gaussian low pass filter spatially, and separately, for exponential moving average temporally,\n"
        "with an equal weighting to the spatial and temporal both filters. Each pixel stores a uniform random\n"
        "scalar value."
    );
}

bool ParseCommandLine(fastnoise::Context::ContextInput& settings, int argc, char** argv)
{
    int nextArg = 1;

    // sampleSpace
    {
        if (nextArg >= argc)
        {
            printf("[Error] sampleSpace missing\n");
            return false;
        }
        const char* arg = argv[nextArg];
        nextArg++;

        if (!_stricmp(arg, "Real"))
            settings.variable_sampleSpace = fastnoise::SampleSpace::Real;
        else if (!_stricmp(arg, "Circle"))
            settings.variable_sampleSpace = fastnoise::SampleSpace::Circle;
        else if (!_stricmp(arg, "Vector2"))
            settings.variable_sampleSpace = fastnoise::SampleSpace::Vector2;
        else if (!_stricmp(arg, "Vector3"))
            settings.variable_sampleSpace = fastnoise::SampleSpace::Vector3;
        else if (!_stricmp(arg, "Vector4"))
            settings.variable_sampleSpace = fastnoise::SampleSpace::Vector4;
        else if (!_stricmp(arg, "Sphere"))
            settings.variable_sampleSpace = fastnoise::SampleSpace::Sphere;
        else
        {
            printf("[Error] Unknown sampleSpace: \"%s\"\n", arg);
            return false;
        }
    }

    // distribution
    {
        if (nextArg >= argc)
        {
            printf("[Error] distribution missing\n");
            return false;
        }
        const char* arg = argv[nextArg];
        nextArg++;

        if (!_stricmp(arg, "Uniform"))
        {
            switch (settings.variable_sampleSpace)
            {
                case fastnoise::SampleSpace::Real:
                case fastnoise::SampleSpace::Circle:
                {
                    settings.variable_sampleDistribution = fastnoise::SampleDistribution::Uniform1D;
                    break;
                }
                case fastnoise::SampleSpace::Vector2:
                {
                    settings.variable_sampleDistribution = fastnoise::SampleDistribution::Uniform2D;
                    break;
                }
                case fastnoise::SampleSpace::Vector3:
                {
                    settings.variable_sampleDistribution = fastnoise::SampleDistribution::Uniform3D;
                    break;
                }
                case fastnoise::SampleSpace::Vector4:
                {
                    settings.variable_sampleDistribution = fastnoise::SampleDistribution::Uniform4D;
                    break;
                }
                case fastnoise::SampleSpace::Sphere:
                {
                    settings.variable_sampleDistribution = fastnoise::SampleDistribution::UniformSphere;
                    break;
                }
                default:
                {
                    printf("[Error] unhandled sample space for uniform distribution\n");
                    return false;
                }
            }
        }
        else if (!_stricmp(arg, "Tent"))
        {
            if (settings.variable_sampleSpace == fastnoise::SampleSpace::Real)
                settings.variable_sampleDistribution = fastnoise::SampleDistribution::Tent1D;
            else
            {
                printf("[Error] Only \"Real\" sampleSpace can be Tent distributed.\n");
                return false;
            }
        }
        else if (!_stricmp(arg, "Gauss"))
        {
            if (settings.variable_sampleSpace == fastnoise::SampleSpace::Real)
                settings.variable_sampleDistribution = fastnoise::SampleDistribution::Gauss1D;
            else
            {
                printf("[Error] Only \"Real\" sampleSpace can be Gauss distributed.\n");
                return false;
            }
        }
        else if (!_stricmp(arg, "Cosine"))
        {
            if (settings.variable_sampleSpace == fastnoise::SampleSpace::Sphere)
                settings.variable_sampleDistribution = fastnoise::SampleDistribution::CosineHemisphere;
            else
            {
                printf("[Error] Only \"Sphere\" sampleSpace can be Cosine distributed.\n");
                return false;
            }
        }
        else if (!_stricmp(arg, "UniformHemisphere"))
        {
            if (settings.variable_sampleSpace == fastnoise::SampleSpace::Sphere)
                settings.variable_sampleDistribution = fastnoise::SampleDistribution::UniformHemisphere;
            else
            {
                printf("[Error] Only \"Sphere\" sampleSpace can be UniformHemisphere distributed.\n");
                return false;
            }
        }
        else
        {
            printf("[Error] Unknown distribution: \"%s\"\n", arg);
            return false;
        }
    }

    // filterXY
    {
        if (nextArg >= argc)
        {
            printf("[Error] filterXY missing\n");
            return false;
        }
        const char* arg = argv[nextArg];
        nextArg++;

        if (!_stricmp(arg, "Box"))
            settings.variable_filterX = fastnoise::FilterType::Box;
        else if (!_stricmp(arg, "Gauss"))
            settings.variable_filterX = fastnoise::FilterType::Gaussian;
        else if (!_stricmp(arg, "Binomial"))
            settings.variable_filterX = fastnoise::FilterType::Binomial;
        else
        {
            printf("[Error] Unknown filterXY type: \"%s\"\n", arg);
            return false;
        }

        if (nextArg >= argc)
        {
            printf("[Error] filterXY parameter missing\n");
            return false;
        }
        arg = argv[nextArg];
        nextArg++;

        sscanf_s(arg, "%f", &settings.variable_filterXparams[0]);

        settings.variable_filterY = settings.variable_filterX;
        settings.variable_filterYparams = settings.variable_filterXparams;
    }

    // filterZ
    {
        if (nextArg >= argc)
        {
            printf("[Error] filterZ missing\n");
            return false;
        }
        const char* arg = argv[nextArg];
        nextArg++;

        if (!_stricmp(arg, "Box"))
            settings.variable_filterZ = fastnoise::FilterType::Box;
        else if (!_stricmp(arg, "Gauss"))
            settings.variable_filterZ = fastnoise::FilterType::Gaussian;
        else if (!_stricmp(arg, "Binomial"))
            settings.variable_filterZ = fastnoise::FilterType::Binomial;
        else if (!_stricmp(arg, "Exponential"))
            settings.variable_filterZ = fastnoise::FilterType::WeightedExponential;
        else
        {
            printf("[Error] Unknown filterZ type: \"%s\"\n", arg);
            return false;
        }

        if (nextArg >= argc)
        {
            printf("[Error] filterZ parameter missing\n");
            return false;
        }
        arg = argv[nextArg];
        nextArg++;

        sscanf_s(arg, "%f", &settings.variable_filterZparams[0]);


        if (settings.variable_filterZ == fastnoise::FilterType::WeightedExponential)
        {
            if (nextArg >= argc)
            {
                printf("[Error] filterZ parameter 2 missing\n");
                return false;
            }
            arg = argv[nextArg];
            nextArg++;

            sscanf_s(arg, "%f", &settings.variable_filterZparams[1]);
        }
    }

    // filterCombine
    {
        if (nextArg >= argc)
        {
            printf("[Error] filterZ missing\n");
            return false;
        }
        const char* arg = argv[nextArg];
        nextArg++;

        if (!_stricmp(arg, "Separate"))
            settings.variable_separate = true;
        else if (!_stricmp(arg, "Product"))
            settings.variable_separate = false;
        else
        {
            printf("[Error] Unknown filterCombine type: \"%s\"\n", arg);
            return false;
        }

        if (settings.variable_separate)
        {
            if (nextArg >= argc)
            {
                printf("[Error] filterCombine spatialWeight missing\n");
                return false;
            }
            arg = argv[nextArg];
            nextArg++;

            sscanf_s(arg, "%f", & settings.variable_separateWeight);
        }
    }

    // textureSize
    {
        for (int i = 0; i < 3; ++i)
        {
            if (nextArg >= argc)
            {
                printf("[Error] textureSize[%i]\n", i);
                return false;
            }
            const char* arg = argv[nextArg];
            nextArg++;

            sscanf_s(arg, "%u", &settings.variable_TextureSize[i]);

            if (settings.variable_TextureSize[i] & (settings.variable_TextureSize[i]-1))
            {
                printf("[Error] textureSize[%i] must be a power of 2, got %i\n", i, settings.variable_TextureSize[i]);
                return false;
            }

        }
    }

    // fileName
    {
        if (nextArg >= argc)
        {
            printf("[Error] fileName missing\n");
            return false;
        }
        g_outputFileName = argv[nextArg];
        nextArg++;
    }

    // optional parameters
    while (nextArg < argc)
    {

        // split
        if (!_stricmp(argv[nextArg], "-split"))
        {
            g_outputLayersAsSingleImages = true;
            nextArg++;
        }
        else if (!_stricmp(argv[nextArg], "-numsteps"))
        {
            nextArg++;
            int numSteps = 0;
            if (nextArg < argc && sscanf_s(argv[nextArg], "%i", &numSteps) == 1)
            {
                g_numSteps = numSteps;
            }
            else
            {
                printf("[Error] -numsteps is missing the number of steps\n");
                return false;
            }
        }
        else
        {
            nextArg++;
        }
    }

    return true;
}

int main(int argc, char** argv)
{
    #if DETERMINISTIC()
    std::mt19937 rng;
    #else
    std::random_device rd;
    std::mt19937 rng(rd());
    #endif
    std::uniform_int_distribution<unsigned int> dist(0);

    // initialize directx
    DX12 dx12;

    // create the context
    fastnoise::Context* fastnoiseContext = nullptr;
    {
        fastnoise::Context::LogFn = &LogFn;
        fastnoise::Context::s_techniqueLocation = L"fastnoise/";
        fastnoiseContext = fastnoise::CreateContext(dx12.m_device);
        if (!fastnoiseContext)
            Assert(false, "Could not create fastnoise context");
        fastnoiseContext->m_profile = false;
    }

    // read the command line
    if (!ParseCommandLine(fastnoiseContext->m_input, argc, argv))
    {
        PrintUsage();
        return 1;
    }

    // random seed
    fastnoiseContext->m_input.variable_rngSeed = dist(rng);
    fastnoiseContext->m_input.variable_scrambleBits = (unsigned int)std::min(std::log2(float(fastnoiseContext->m_input.variable_TextureSize[0])), std::log2(float(fastnoiseContext->m_input.variable_TextureSize[0])));

    fastnoiseContext->m_input.variable_swapSuppression = 8;

    // 
    SBuffer<float> filterBuffer;
    {
        std::vector<float> filterData;

        fastnoise::FilterType filterTypes[3] = { fastnoiseContext->m_input.variable_filterX, fastnoiseContext->m_input.variable_filterY, fastnoiseContext->m_input.variable_filterZ };
        fastnoise::float4 filterParams[3] = { fastnoiseContext->m_input.variable_filterXparams, fastnoiseContext->m_input.variable_filterYparams, fastnoiseContext->m_input.variable_filterZparams };

        for (int c = 0; c < 3; c++)
        {
            fastnoise::FilterType filterType = filterTypes[c];
            fastnoise::float4 filterParam = filterParams[c];

            switch (filterType)
            {
            case fastnoise::FilterType::Box:

            {
                int boxFilterSize = (int)filterParam[0];
                Assert(boxFilterSize > 0, "Box filter parameter 0 (boxFilterSize) must be positive");

                #if 1
                fastnoiseContext->m_input.variable_filterMin[c] = -(boxFilterSize - 1);
                fastnoiseContext->m_input.variable_filterMax[c] = boxFilterSize - 1;
                fastnoiseContext->m_input.variable_filterOffset[c] = (int)(filterData.size() + boxFilterSize - 1);

                for (int i = -(boxFilterSize - 1); i <= boxFilterSize - 1; i++)
                {
                    float filterValue = std::max<float>(0.0f, float(boxFilterSize - abs(i))) / (boxFilterSize * boxFilterSize);
                    filterData.push_back(filterValue);
                }

                #else
                // For small textures, this can be desirable. Otherwise the filter would get truncated, which results in an error below.

                fastnoiseContext->m_input.variable_filterMin[c] = -boxFilterSize / 2;
                fastnoiseContext->m_input.variable_filterMax[c] = fastnoiseContext->m_input.variable_filterMin[c] + boxFilterSize - 1;
                fastnoiseContext->m_input.variable_filterOffset[c] = (int)(filterData.size() + boxFilterSize - 1);

                for (int i = 0; i < boxFilterSize; ++i)
                    filterData.push_back(1.0f / float(boxFilterSize));

                #endif

                break;
            }

            case fastnoise::FilterType::Binomial:

            {
                int binomialFilterSize = (int)filterParam[0];
                Assert(binomialFilterSize > 0, "Binomial filter parameter 0 (binomialFilterSize) must be positive");

                fastnoiseContext->m_input.variable_filterMin[c] = -binomialFilterSize;
                fastnoiseContext->m_input.variable_filterMax[c] = binomialFilterSize;
                fastnoiseContext->m_input.variable_filterOffset[c] = (int)(filterData.size() + binomialFilterSize);

                // Precomputed normalization of the filter
                float filterPow = pow(0.5f, 2.0f * float(binomialFilterSize));

                for (int i = -binomialFilterSize; i <= binomialFilterSize; i++)
                {
                    // Calculate r = n choose k
                    int n = 2 * binomialFilterSize;
                    int k = binomialFilterSize - i;
                    float r = 1.0f;
                    for (int j = 0; j < k; j++) {
                        r *= float(n - j) / float(k - j);
                    }
                    float filterValue = filterPow * r;

                    filterData.push_back(filterValue);
                }

                break;
            }

            case fastnoise::FilterType::Gaussian:

            {
                float sigma = filterParam[0];

                static const float c_energy = 0.995f;

                // Construct the filter f before doubling and apply a cutoff to that.
                // This avoids negative lobes in the doubled filter.
                float scale = sqrtf(0.5f) / sigma;
                float total = 0.0f;
                std::vector<float> filter;
                for (int i = 0; total < c_energy; ++i)
                {
                    float filterVal = 0.5f * (erff(scale * (i + 0.5f)) - erff(scale * (i - 0.5f)));
                    filter.push_back(filterVal);
                    total += (i > 0 ? 2.0f : 1.0f) * filterVal;
                }

                // Determines the min/max extents of the filter (inclusive)
                int filterSize = 2 * ((int)filter.size() - 1);

                fastnoiseContext->m_input.variable_filterMin[c] = -filterSize;
                fastnoiseContext->m_input.variable_filterMax[c] = filterSize;
                fastnoiseContext->m_input.variable_filterOffset[c] = (int)filterData.size() + filterSize;

                // Calculate the convolution of the filter f with itself
                for (int i = -filterSize; i <= filterSize; i++)
                {
                    float filterValue = 0.0f;
                    for (int j = std::max(0, i) - (int)filter.size() + 1; j < (int)filter.size() + std::min(0, i); ++j)
                    {
                        filterValue += filter[abs(j)] * filter[abs(i - j)];
                    }
                    filterData.push_back(filterValue);

                }

                break;
            }

            case fastnoise::FilterType::WeightedExponential:

            {
                float alpha = filterParam[0];
                float beta = filterParam[1];
                int size = fastnoiseContext->m_input.variable_TextureSize[c];
                int offset = (int)filterData.size();

                // For temporal filter, start offset at zero 
                fastnoiseContext->m_input.variable_filterMin[c] = 0;
                fastnoiseContext->m_input.variable_filterMax[c] = size-1;
                fastnoiseContext->m_input.variable_filterOffset[c] = offset;

                filterData.insert(filterData.end(), size, 0.0f);

                // m is a randomly chosen cutoff on the temporal filter
                for (int m = 1; m <= size; m++)
                {
                    float weight = beta > 0.0f ? beta * pow(1.0f - beta, (float)(m - 1)) / (1.0f - pow(1.0f - beta, (float)size)) : 1.0f/size;

                    for (int j = -m + 1; j < m; j++)
                    {
                        float Fj = 0.0f;
                        for (int i = std::max(0, -j); i < std::min(m, m - j); i++)
                        {
                            float fi = pow(1.0f - alpha, float(i));
                            if (i < m - 1) fi *= alpha;

                            float fij = pow(1.0f - alpha, float(i + j));
                            if (i + j < m - 1) fij *= alpha;

                            Fj += fi * fij;
                        }

                        // Allow filter to wrap
                        int wrappedJ = ((j % size) + size) % size;
                        filterData[offset + wrappedJ] += Fj * weight;
                    }
                }

                break;
            }

            default:
                Assert(false, "Unimplemented filter type %i", filterType);

                break;

            }
        }

        filterBuffer.Load(dx12.m_device, &filterData[0], filterData.size(), "Filter Buffer");

        // If the filter is larger than the image on any axis, that is an error condition.
        // Exponential filtering is an exception to this
        for (int c = 0; c < 3; c++)
        {
            if (filterTypes[c] == fastnoise::FilterType::WeightedExponential || filterTypes[c] == fastnoise::FilterType::Exponential)
                continue;

            if ((int)fastnoiseContext->m_input.variable_TextureSize[c] < 1 + fastnoiseContext->m_input.variable_filterMax[c] - fastnoiseContext->m_input.variable_filterMin[c])
            {
                printf("[Error] Filter Truncation: filter on axis %i is of size %i, but the texture is only %i.\n", c, 1 + fastnoiseContext->m_input.variable_filterMax[c] - fastnoiseContext->m_input.variable_filterMin[c], (int)fastnoiseContext->m_input.variable_TextureSize[c]);
                return 1;
            }
        }
    }

    // Iterate
    {
        const size_t c_imageReadbackInterval = std::max<size_t>(g_numSteps / 10, 1);
        const size_t c_statusReportInterval = std::max<size_t>(g_numSteps / 100, 1);

        SImage fastnoiseTexture;
        SBuffer<fastnoise::Struct_DataStruct> fastnoiseData;
        for (int step = 0; step < g_numSteps; ++step)
        {
            #if OUTPUT_INTERMEDIATE_IMAGES()
            bool readbackImage = ((step % c_imageReadbackInterval) == 0) || step == (g_numSteps - 1);
            #else
            bool readbackImage = step == (g_numSteps - 1);
            #endif

            bool readbackBuffer = ((step % c_statusReportInterval) == 0) || step == (g_numSteps - 1);

            // DEBUG: output every image
            //readbackImage = true;

            dx12.Execute(
                [&](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
                {
                    fastnoise::OnNewFrame(1);

                    fastnoiseContext->m_input.variable_Iteration = step;

                    // Set up key for Feistel network
                    fastnoiseContext->m_input.variable_key[0] = dist(rng);
                    fastnoiseContext->m_input.variable_key[1] = dist(rng);
                    fastnoiseContext->m_input.variable_key[2] = dist(rng);
                    fastnoiseContext->m_input.variable_key[3] = dist(rng);

                    if (step == 0)
                    {
                        filterBuffer.UploadDataToGPU(device, cmdList);

                        fastnoiseContext->m_input.buffer_Filter = filterBuffer.m_resource;
                        fastnoiseContext->m_input.buffer_Filter_stride = 0;
                        fastnoiseContext->m_input.buffer_Filter_format = DXGI_FORMAT_R32_FLOAT;
                        fastnoiseContext->m_input.buffer_Filter_count = (unsigned int)filterBuffer.m_data.size();
                        fastnoiseContext->m_input.buffer_Filter_state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

                    }

                    fastnoise::Execute(fastnoiseContext, device, cmdList);

                    if (step == 0)
                    {
                        fastnoiseTexture.AdoptResource(fastnoiseContext->m_output.texture_Texture, fastnoiseContext->m_input.variable_TextureSize[0], fastnoiseContext->m_input.variable_TextureSize[1] * fastnoiseContext->m_input.variable_TextureSize[2], 4, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(float));
                        fastnoiseData.AdoptResource(fastnoiseContext->m_output.buffer_Data, fastnoiseContext->m_output.buffer_Data_count);
                    }

                    if (readbackImage)
                    {
                        if (fastnoiseContext->m_output.c_texture_Texture_endingState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
                        {
                            D3D12_RESOURCE_BARRIER barrier;
                            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                            barrier.Transition.pResource = fastnoiseContext->m_output.texture_Texture;
                            barrier.Transition.StateBefore = fastnoiseContext->m_output.c_texture_Texture_endingState;
                            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                            cmdList->ResourceBarrier(1, &barrier);
                        }

                        fastnoiseTexture.RequestReadback(device, cmdList);

                        if (fastnoiseContext->m_output.c_texture_Texture_endingState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
                        {
                            D3D12_RESOURCE_BARRIER barrier;
                            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                            barrier.Transition.pResource = fastnoiseContext->m_output.texture_Texture;
                            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                            barrier.Transition.StateAfter = fastnoiseContext->m_output.c_texture_Texture_endingState;
                            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                            cmdList->ResourceBarrier(1, &barrier);
                        }
                    }

                    if (readbackBuffer)
                    {
                        if (fastnoiseContext->m_output.c_buffer_Data_endingState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
                        {
                            D3D12_RESOURCE_BARRIER barrier;
                            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                            barrier.Transition.pResource = fastnoiseContext->m_output.buffer_Data;
                            barrier.Transition.StateBefore = fastnoiseContext->m_output.c_buffer_Data_endingState;
                            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                            cmdList->ResourceBarrier(1, &barrier);
                        }

                        fastnoiseData.RequestReadback(device, cmdList);

                        if (fastnoiseContext->m_output.c_buffer_Data_endingState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
                        {
                            D3D12_RESOURCE_BARRIER barrier;
                            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                            barrier.Transition.pResource = fastnoiseContext->m_output.buffer_Data;
                            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                            barrier.Transition.StateAfter = fastnoiseContext->m_output.c_buffer_Data_endingState;
                            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                            cmdList->ResourceBarrier(1, &barrier);
                        }
                    }
                }
            );

            #if OUTPUT_INTERMEDIATE_IMAGES()
            if (readbackImage)
            {
                char fileName[256];
                if (step == (g_numSteps - 1))
                {
                    sprintf_s(fileName, "%s.png", g_outputFileName.c_str());
                }
                else
                {
                    sprintf_s(fileName, "%s_%i.png", g_outputFileName.c_str(), step);
                }
                fastnoiseTexture.DoReadback();
                fastnoiseTexture.Save(fileName, true);
            }
            #else
            if (readbackImage)
            {
                char fileName[256];
                fastnoiseTexture.DoReadback();

                //g_outputLayersAsSingleImages

                const char* extension = "png";
                SImage::PixelConversions pixelConversion = SImage::PixelConversions::PixelsAreF32_SaveAsU8;

                if (fastnoiseContext->m_input.variable_sampleDistribution == fastnoise::SampleDistribution::Gauss1D)
                {
                    extension = "hdr";
                    pixelConversion = SImage::PixelConversions::PixelsAreF32_SaveAsF32;
                }

                if (g_outputLayersAsSingleImages && fastnoiseContext->m_input.variable_TextureSize[2] > 1)
                {
                    for (unsigned int z = 0; z < fastnoiseContext->m_input.variable_TextureSize[2]; ++z)
                    {
                        sprintf_s(fileName, "%s_%i.%s", g_outputFileName.c_str(), z, extension);
                        fastnoiseTexture.SaveRegion(fileName, 0, fastnoiseContext->m_input.variable_TextureSize[0], z * fastnoiseContext->m_input.variable_TextureSize[1], (z + 1) * fastnoiseContext->m_input.variable_TextureSize[1], pixelConversion);
                    }
                }
                else
                {
                    sprintf_s(fileName, "%s.%s", g_outputFileName.c_str(), extension);
                    fastnoiseTexture.Save(fileName, pixelConversion);
                }
            }
            #endif

            if (readbackBuffer)
            {
                fastnoiseData.DoReadback();
                float percent = 100.0f * float(step) / float(g_numSteps - 1);
                printf("\r%0.2f%%  iterations = %i, swaps = %i, suppression = %i\n", percent, step, fastnoiseData.m_data[0].swaps, fastnoiseContext->m_input.variable_swapSuppression);

                char buffer[1024];
                sprintf_s(buffer, "%s [%i%%]", g_outputFileName.c_str(), int(percent));
                SetConsoleTitleA(buffer);

                // Dynamic swap suppression. If the number of swaps is lower than expected we can reduce the suppression rate.
                if (fastnoiseContext->m_input.variable_swapSuppression > 1)
                {
                    unsigned int pixels = fastnoiseContext->m_input.variable_TextureSize[0] * fastnoiseContext->m_input.variable_TextureSize[1] * fastnoiseContext->m_input.variable_TextureSize[2];
                    if (8 * fastnoiseData.m_data[0].swaps * fastnoiseContext->m_input.variable_swapSuppression < pixels)
                    {
                        fastnoiseContext->m_input.variable_swapSuppression /= 2;
                    }
                }

            }

            // This is how you'd get or print out cpu and gpu profiling info if you want it.
            // You would need to set fastnoiseContext->m_profile to true though when you create the context, instead of false.
            /*
            int numItems = 0;
            auto* items = fastnoiseContext->ReadbackProfileData(dx12.m_commandQueue, numItems);
            for (int i = 0; i < numItems; ++i)
                printf("fastnoise::%s\tcpu=%0.2fms\tgpu=%0.2fms\n", items[i].m_label, items[i].m_cpu * 1000.0f, items[i].m_gpu * 1000.0f);
            */
        }
    }

    // Shutdown
    fastnoise::DestroyContext(fastnoiseContext);
    printf("\n\n");

    return 0;
}
