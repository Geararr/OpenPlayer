#include "utils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include <random>
#include <filesystem>
#include <slint/slint.h>

#include <cstring>

slint::Image CreateNoiseImage(const uint32_t width = 128, const uint32_t height = 128) {
    slint::SharedPixelBuffer<slint::Rgba8Pixel> pixelBuffer(width, height);

    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 255);

    for (auto& pixel : pixelBuffer) {
        pixel.r = static_cast<uint8_t>(dist(rng));
        pixel.g = static_cast<uint8_t>(dist(rng));
        pixel.b = static_cast<uint8_t>(dist(rng));
        pixel.a = 255;
    }

    return {pixelBuffer};
}

std::vector<uint8_t> SoundData::GetAudioCoverBytes(const std::filesystem::path& filePath)
{
    std::vector<uint8_t> imageBytes;
    if (filePath.empty()) return imageBytes;

    const TagLib::FileRef f(filePath.c_str());

    if (f.isNull() || !f.file() || !f.file()->isValid()) return imageBytes;

    const std::string filename = filePath.filename().string();

    TagLib::File* filePtr = f.file();

    std::string ext = filePath.extension().string();
    std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return std::tolower(c); });

    // mp3
    if (ext == ".mp3")
    {

        if (auto* mpegFile = dynamic_cast<TagLib::MPEG::File*>(f.file()))
        {
            auto* id3v2Tag = mpegFile->ID3v2Tag();
            if (id3v2Tag)
            {
                const auto& frames = id3v2Tag->frameList("APIC");
                if (!frames.isEmpty()) {
                    TagLib::ID3v2::AttachedPictureFrame* bestFrame = nullptr;

                    for (auto* frame : frames) {
                        auto* picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame);
                        if (picFrame) {
                            if (picFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover) {
                                bestFrame = picFrame;
                                break;
                            }
                            if (!bestFrame) {
                                bestFrame = picFrame;
                            }
                        }
                    }

                    if (bestFrame) {
                        #if TAGLIB_VERSION >= 0x020000
                            auto dataView = bestFrame->picture();
                            imageBytes.assign(dataView.begin(), dataView.end());
                        #else
                            const TagLib::ByteVector& data = bestFrame->picture();
                            imageBytes.assign(data.data(), data.data() + data.size());
                        #endif
                    }
                }
            }
        }
    }

    // mp4
    else if (auto* mp4File = dynamic_cast<TagLib::MP4::File*>(filePtr))
    {
        std::cout << "Getting audio cover bytes for mp4 -> " << filename << '\n';
        if (mp4File->tag())
        {
            auto itemMap = mp4File->tag()->itemMap();
            if (itemMap.contains("covr"))
            {
                auto coverList = itemMap["covr"].toCoverArtList();
                if (!coverList.isEmpty())
                {
                    std::cout << "Cover list not empty\n";
                    #if TAGLIB_VERSION >= 0x020000
                        auto dataView = coverList.front().image();
                        imageBytes.assign(dataView.begin(), dataView.end());
                    #else
                        const TagLib::ByteVector& data = coverList.front().data();
                        imageBytes.assign(data.data(), data.data() + data.size());
                    #endif
                }
            }
        }
    }
    return imageBytes;
}

slint::Image SoundData::GetImageFromBytes(const std::vector<uint8_t>& imageBytes, const int maxDimension)
{
    if (imageBytes.empty()) {
        return CreateNoiseImage();
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    if (!stbi_info_from_memory(imageBytes.data(), static_cast<int>(imageBytes.size()), &width, &height, &channels)) {
        std::cerr << "Failed to decode image data!\n";
        return CreateNoiseImage();
    }
    if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
        std::cerr << "Image dimensions are too large!\n";
        return CreateNoiseImage();
    }

    unsigned char* decoded_pixels = stbi_load_from_memory(
        imageBytes.data(),
        static_cast<int>(imageBytes.size()),
        &width,
        &height,
        &channels,
        4
    );

    if (!decoded_pixels) {
        std::cerr << "Failed to decode image: " << stbi_failure_reason() << '\n';
        return CreateNoiseImage();
    }

    if (width > maxDimension || height > maxDimension) {
        const float scale = static_cast<float>(maxDimension) / std::max(width, height);
        const int targetW = std::max(1, static_cast<int>(width * scale));
        const int targetH = std::max(1, static_cast<int>(height * scale));

        slint::SharedPixelBuffer<slint::Rgba8Pixel> pixel_buffer(targetW, targetH);

        void* result = stbir_resize_uint8_srgb(
            decoded_pixels, width, height, 0,
            reinterpret_cast<unsigned char *>(pixel_buffer.begin()), targetW, targetH, 0,
            STBIR_RGBA
        );

        if (!result) {
            std::cerr << "Image resizing failed!\n";
            return CreateNoiseImage();
        }

        stbi_image_free(decoded_pixels);
        return slint::Image(pixel_buffer);
    }

    slint::SharedPixelBuffer<slint::Rgba8Pixel> pixel_buffer(width, height);

    const size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    std::memcpy(pixel_buffer.begin(), decoded_pixels, byteCount);

    stbi_image_free(decoded_pixels);
    return slint::Image(pixel_buffer);
}

std::filesystem::path SoundData::getExecDir() {
#ifdef __EMSCRIPTEN__
    return "/";
#else
    #ifdef _WIN32
        wchar_t path[MAX_PATH] = { 0 };
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        return std::filesystem::path(path).parent_path();
    #else
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        return std::filesystem::path(std::string(result, (count > 0) ? count : 0)).parent_path();
    #endif
#endif
}
