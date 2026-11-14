#pragma once

#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4item.h>

#include <slint/slint.h>

#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>

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
    std::vector<uint8_t> GetAudioCoverBytes(const std::string& filePath);
    slint::Image GetImageFromBytes(const std::vector<uint8_t>& imageBytes);
    std::filesystem::path getExecDir();
}