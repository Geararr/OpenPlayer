#include "utils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <slint/slint.h>

unsigned long long Resources::GetAvailableMemory()
{
        #if defined(_WIN32) || defined(_WIN64)
            MEMORYSTATUSEX memInfo;
            memInfo.dwLength = sizeof(MEMORYSTATUSEX);
            if (GlobalMemoryStatusEx(&memInfo))
            {
                return memInfo.ullAvailPhys;
            }
        #elif defined(__linux__)
            struct sysinfo memInfo;
            if (sysinfo(&memInfo) == 0)
            {
                return (unsigned long long)memInfo.freeram * memInfo.mem_unit;
            }
        #endif
            return 0;
}

std::vector<uint8_t> SoundData::GetAudioCoverBytes(const std::string& filePath)
{
    std::vector<uint8_t> imageBytes;

    TagLib::MPEG::File file(filePath.c_str());
    if (!file.isValid()) return imageBytes;

    TagLib::ID3v2::Tag* id3v2Tag = file.ID3v2Tag();
    if (!id3v2Tag) return imageBytes;

    const auto& frames = id3v2Tag->frameList("APIC");
    if (frames.isEmpty()) return imageBytes;

    for (auto* frame: frames)
    {
        auto* picFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame);
        if (picFrame && picFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
        {
            #if TAGLIB_VERSION >= 0x020000
                auto dataView = picFrame->picture();
                imageBytes.assign(dataView.begin(), dataView.end());
            #else 
                const TagLib::ByteVector& pictureData = picFrame->picture();
                imageBytes.assign(pictureData.data(), pictureData.data() + pictureData.size());
            #endif
            break;
        }
    }

    return imageBytes;
}

slint::Image SoundData::GetImageFromBytes(const std::vector<uint8_t>& imageBytes)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* decoded_pixels = stbi_load_from_memory(
        imageBytes.data(),
        static_cast<int>(imageBytes.size()),
        &width,
        &height,
        &channels,
        4
    );

    if (!decoded_pixels) {
        std::cout << "Failed to decode compressed image data!\n";
        return slint::Image();
    }

    slint::SharedPixelBuffer<slint::Rgba8Pixel> pixel_buffer(width, height);

    std::copy(
        reinterpret_cast<const slint::Rgba8Pixel*>(decoded_pixels),
        reinterpret_cast<const slint::Rgba8Pixel*>(decoded_pixels) + (width * height),
        pixel_buffer.begin()
    );

    stbi_image_free(decoded_pixels);

    return slint::Image(pixel_buffer);
}
