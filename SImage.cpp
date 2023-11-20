///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS

#define TINYEXR_IMPLEMENTATION
#include "tinyexr/tinyexr.h"

#include "SImage.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

static const char* GetFileExtension(const char* fileName)
{
    int index = (int)strlen(fileName);
    while (index >= 0)
    {
        if (fileName[index] == '.')
            return &fileName[index];
        index--;
    }
    return nullptr;
}

static bool SaveCSV(const float* pixels, int width, int height, int numChannels, const char* outfilename)
{
    FILE* file = nullptr;
    fopen_s(&file, outfilename, "wb");
    if (!file)
        return false;

    const float* value = pixels;
    for (int iy = 0; iy < height; ++iy)
    {
        for (int ix = 0; ix < width * numChannels; ++ix)
        {
            fprintf(file, "%s\"%f\"", ix > 0 ? "," : "", *value);
            value++;
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return true;
}

static bool SaveEXR(const float* pixels, int width, int height, int numChannels, const char* outfilename)
{
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = numChannels;

    std::vector<std::vector<float>> images;
    images.resize(numChannels);
    for (std::vector<float>& v : images)
        v.resize(width * height);

    // Split interleaved channels into separate channels
    for (int i = 0; i < width * height; i++)
    {
        for (int j = 0; j < numChannels; ++j)
            images[j][i] = pixels[numChannels * i + j];
    }

    // put the RGBA channels into ABGR because most EXR viewers expect this
    std::vector<float*> image_ptr(numChannels);
    for (int i = 0; i < numChannels; ++i)
        image_ptr[i] = images[numChannels - i - 1].data();

    image.images = (unsigned char**)image_ptr.data();
    image.width = width;
    image.height = height;

    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    const char* chanelNames[4] = { "R", "G", "B", "A" };
    header.num_channels = numChannels;
    header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    for (int i = 0; i < numChannels; ++i)
    {
        char buffer[256];
        const char* channelName;
        if (i < 4)
        {
            channelName = chanelNames[numChannels - i - 1];
        }
        else
        {
            sprintf(buffer, "X%i", i - 4);
            channelName = buffer;
        }
        strncpy(header.channels[i].name, channelName, 255); header.channels[0].name[strlen(channelName)] = '\0';
    }

    header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;// TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    const char* err = NULL; // or nullptr in C++11 or later.
    int ret = SaveEXRImageToFile(&image, &header, outfilename, &err);
    if (ret != TINYEXR_SUCCESS) {
        //fprintf(stderr, "Save EXR err: %s\n", err);
        FreeEXRErrorMessage(err); // free's buffer for an error message
        return ret;
    }
    //printf("Saved exr file. [ %s ] \n", outfilename);

    //free(pixels);

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);

    return true;
}

SImage::~SImage()
{
    if (m_releaseResource && m_resource)
        m_resource->Release();
    for (ID3D12Resource* resource : m_uploadBuffers)
        resource->Release();
    if (m_readbackBuffer)
        m_readbackBuffer->Release();
}

bool SImage::Load(ID3D12Device* device, const char* fileName)
{
    m_debugName = std::wstring(CA2W(std::string(fileName).c_str()));

    // load the file into memory.
    // Supports 1, 2, 3 or 4 components per pixel.
    // 3 component images are given an alpha channel and then become 4 components per pixel
    unsigned char* pixels = stbi_load(fileName, &m_width, &m_height, &m_components, 0);
    if (!pixels)
        return false;
    if (m_components == 3)
    {
        m_components = 4;
        m_pixels.resize(m_width * m_height * m_components, 255);
        for (int index = 0; index < m_width * m_height; ++index)
        {
            m_pixels[index * 4 + 0] = pixels[index * 3 + 0];
            m_pixels[index * 4 + 1] = pixels[index * 3 + 1];
            m_pixels[index * 4 + 2] = pixels[index * 3 + 2];
        }
    }
    else
    {
        m_pixels.resize(m_width * m_height * m_components);
        memcpy(m_pixels.data(), pixels, m_pixels.size());
    }
    stbi_image_free(pixels);

    // figure out the format to use
    switch (m_components)
    {
        case 1: m_format = DXGI_FORMAT_R8_UNORM; break;
        case 2: m_format = DXGI_FORMAT_R8G8_UNORM; break;
        case 4: m_format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
        default: return false;
    }

    // Create a resource
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = m_format;
    textureDesc.Width = m_width;
    textureDesc.Height = m_height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&m_resource));

    // set the name for debugging purposes
    m_resource->SetName(m_debugName.c_str());

    return true;
}

bool SImage::Save(const char* fileName, PixelConversions pixelConversion)
{
    switch (pixelConversion)
    {
        case PixelConversions::PixelsAreF32_SaveAsU8:
        {
            // NOTE: this DOES NOT convert from linear to sRGB

            const size_t count = m_pixels.size() / sizeof(float);

            m_F23ToU8pixels.resize(count);

            const float* src = (const float*)m_pixels.data();
            unsigned char* dest = m_F23ToU8pixels.data();
            for (size_t index = 0; index < count; ++index)
                dest[index] = (unsigned char)std::max(std::min(src[index] * 256.0f, 255.0f), 0.0f);

            return stbi_write_png(fileName, m_width, m_height, m_components, m_F23ToU8pixels.data(), 0) == 1;
        }
        case PixelConversions::PixelsAreU8_SaveAsU8:
        {
            return stbi_write_png(fileName, m_width, m_height, m_components, m_pixels.data(), 0) == 1;
        }
        case PixelConversions::PixelsAreF32_SaveAsF32:
        {
            const char* extension = GetFileExtension(fileName);
            if (!_stricmp(extension, ".exr"))
                return SaveEXR((float*)m_pixels.data(), m_width, m_height, m_components, fileName);
            else if (!_stricmp(extension, ".csv"))
                return SaveCSV((float*)m_pixels.data(), m_width, m_height, m_components, fileName);
            else
                return stbi_write_hdr(fileName, m_width, m_height, m_components, (float*)m_pixels.data()) == 1;
        }
    }

    return false;
}

bool SImage::SaveRegion(const char* fileName, int x1, int x2, int y1, int y2, PixelConversions pixelConversion)
{
    int regionSize[2] = {
        x2 - x1,
        y2 - y1
    };

    switch (pixelConversion)
    {
        case PixelConversions::PixelsAreF32_SaveAsU8:
        {
            // NOTE: this DOES NOT convert from linear to sRGB

            std::vector<unsigned char> outPixels(regionSize[0] * regionSize[1] * m_components);
            unsigned char* dest = outPixels.data();

            for (int iy = 0; iy < regionSize[1]; ++iy)
            {
                const float* src = (const float*)&m_pixels[((iy + y1) * m_width + x1) * m_components * sizeof(float)];
                for (int ix = 0; ix < regionSize[0] * m_components; ++ix)
                {
                    *dest = (unsigned char)std::max(std::min(*src * 256.0f, 255.0f), 0.0f);
                    src++;
                    dest++;
                }
            }

            return stbi_write_png(fileName, regionSize[0], regionSize[1], m_components, outPixels.data(), 0) == 1;
        }
        case PixelConversions::PixelsAreU8_SaveAsU8:
        {
            std::vector<unsigned char> outPixels(regionSize[0] * regionSize[1] * m_components);
            unsigned char* dest = outPixels.data();

            for (int iy = 0; iy < regionSize[1]; ++iy)
            {
                const unsigned char* src = &m_pixels[((iy + y1) * m_width + x1) * m_components];
                memcpy(dest, src, regionSize[0] * m_components);
                dest += regionSize[0] * m_components;
            }

            return stbi_write_png(fileName, regionSize[0], regionSize[1], m_components, outPixels.data(), 0) == 1;
        }
        case PixelConversions::PixelsAreF32_SaveAsF32:
        {
            std::vector<float> outPixels(regionSize[0] * regionSize[1] * m_components);
            float* dest = outPixels.data();

            for (int iy = 0; iy < regionSize[1]; ++iy)
            {
                const float* src = (const float*)&m_pixels[((iy + y1) * m_width + x1) * m_components * sizeof(float)];
                for (int ix = 0; ix < regionSize[0] * m_components; ++ix)
                {
                    *dest = *src;
                    src++;
                    dest++;
                }
            }

            const char* extension = GetFileExtension(fileName);
            if (!_stricmp(extension, ".exr"))
                return SaveEXR(outPixels.data(), regionSize[0], regionSize[1], m_components, fileName);
            else if(!_stricmp(extension, ".csv"))
                return SaveCSV(outPixels.data(), regionSize[0], regionSize[1], m_components, fileName);
            else
                return stbi_write_hdr(fileName, regionSize[0], regionSize[1], m_components, outPixels.data()) == 1;
        }
    }

    return false;
}

void SImage::UploadPixelsToGPU(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    // create an upload heap resource
    ID3D12Resource* uploadResource = nullptr;
    {
        UINT64 size = 0;
        D3D12_RESOURCE_DESC desc = m_resource->GetDesc();
        device->GetCopyableFootprints(&desc, 0, 1, 0, nullptr, nullptr, nullptr, &size);

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

    // map the upload heap resource and copy the pixels into it
    {
        unsigned char* gpuTextureData = nullptr;
        ThrowIfFailed(uploadResource->Map(0, nullptr, reinterpret_cast<void**>(&gpuTextureData)));
        memcpy(gpuTextureData, m_pixels.data(), m_pixels.size());
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

    // copy the pixel data
    {
        D3D12_RESOURCE_DESC resourceDesc = m_resource->GetDesc();
        std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();
        device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = uploadResource;
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint = *layout;

        D3D12_TEXTURE_COPY_LOCATION dest = {};
        dest.pResource = m_resource;
        dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dest.SubresourceIndex = 0;

        cmdList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
    }

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
};

void SImage::RequestReadback(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    // create a readback heap resource if it doesn't yet exist
    if (!m_readbackBuffer)
    {
        UINT64 size = 0;
        D3D12_RESOURCE_DESC desc = m_resource->GetDesc();
        device->GetCopyableFootprints(&desc, 0, 1, 0, nullptr, nullptr, &m_readbackBufferRowBytes, &size);

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
        barrier.Transition.pResource = m_resource;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &barrier);
    }

    // copy the pixel data
    {
        D3D12_RESOURCE_DESC resourceDesc = m_resource->GetDesc();
        std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();
        device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = m_resource;
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION dest = {};
        dest.pResource = m_readbackBuffer;
        dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dest.PlacedFootprint = *layout;

        m_readbackBufferRowPitch = layout->Footprint.RowPitch;

        cmdList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
    }

    // Transition resource from copy source to unordered access
    {
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_resource;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &barrier);
    }
}

void SImage::DoReadback()
{
    unsigned char* gpuTextureData = nullptr;
    ThrowIfFailed(m_readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&gpuTextureData)));

    // copy each row one by one, to handle pitch
    for (int iy = 0; iy < m_height; ++iy)
    {
        const unsigned char* src = &gpuTextureData[iy * m_readbackBufferRowPitch];
        unsigned char* dest = &m_pixels[iy * m_readbackBufferRowBytes];
        memcpy(dest, src, m_readbackBufferRowBytes);
    }

    m_readbackBuffer->Unmap(0, nullptr);
}
