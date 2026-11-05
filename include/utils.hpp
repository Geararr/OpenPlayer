#pragma once

#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

#include <slint/slint.h>

#include <vector>
#include <cstdint>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>

#elif defined(__linux__)
    #include <sys/sysinfo.h>

#endif

namespace Resources {
    unsigned long long GetAvailableMemory();
}

namespace SoundData {
    std::vector<uint8_t> GetAudioCoverBytes(const std::string& filePath);
    slint::Image GetImageFromBytes(const std::vector<uint8_t>& imageBytes);
}