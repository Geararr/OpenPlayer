#include "utils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <slint/slint.h>

std::vector<uint8_t> SoundData::GetAudioCoverBytes(const std::string& filePath)
{
    const size_t filenamePos = filePath.find_last_of('/');
    const std::string filename = (filenamePos != std::string::npos) ? filePath.substr(filenamePos + 1) : filePath;

    std::vector<uint8_t> imageBytes;

    TagLib::FileRef f(filePath.c_str());
    if (f.isNull() || !f.file()->isValid()) return imageBytes;

    TagLib::File* filePtr = f.file();

    // mp3
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".mp3")
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
        std::cerr << "Failed to decode compressed image data!\n";
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

std::filesystem::path SoundData::getExecDir() {
    #ifdef _WIN32
        wchar_t path[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, path, MAX_PATH);
        return std::filesystem::path(path).parent_path();
    #else
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        return std::filesystem::path(std::string(result, (count > 0) ? count : 0)).parent_path();
    #endif
}
