#pragma once

#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/wavfile.h>
#include <taglib/flacfile.h>
#include <taglib/mp4file.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <algorithm>
#include <cctype>

#include <vips/vips.h>
#include <vips/vips8>

#include <slint/slint.h>

#include <vector>
#include <cstdint>
#include <iostream>

#include "jsonManager.hpp"

#include <filesystem>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <climits>
#endif

namespace SoundData {
    inline CacheManager* cacheManager_;

    void setCacheManager(CacheManager* cacheManager);

    class LruImageCache {
    private:
        struct Entry {
            std::string key;
            slint::SharedPixelBuffer<slint::Rgba8Pixel> buffer;
            size_t bytes;
        };

        size_t maxBytes_;
        size_t currentBytes_ = 0;
        std::list<Entry> items_;
        std::unordered_map<std::string, std::list<Entry>::iterator> map_;

    public:
        explicit LruImageCache(const size_t maxBytes) : maxBytes_(maxBytes) {}

        bool get(const std::string& key, slint::Image& outImage);
        slint::Image put(const std::string& key, slint::SharedPixelBuffer<slint::Rgba8Pixel> buffer);
    };

    std::vector<uint8_t> GetAudioCoverBytes(const std::string& assetKey, const std::filesystem::path& filePath);
    slint::Image GetImageFromBytes(const std::string& assetKey, int maxDimension, const std::filesystem::path& filePath);
    std::filesystem::path getExecDir();
}

slint::Color colorFromHex(std::string_view hex);