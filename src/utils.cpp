#include "utils.hpp"

#include <random>
#include <unordered_map>
#include <filesystem>
#include <slint.h>
#include <malloc.h>

#include <cstring>
#include <cpr/api.h>
#include <cpr/response.h>

using PixelBuffer = slint::SharedPixelBuffer<slint::Rgba8Pixel>;
using AudioCoverBytes = std::vector<uint8_t>;

void SoundData::setCacheManager(CacheManager *cacheManager) {
    cacheManager_ = cacheManager;
}
void SoundData::setConfigManager(ConfigManager *configManager) {
    configManager_ = configManager;
}

bool SoundData::LruImageCache::get(const std::string& key, slint::Image& outImage) {
    const auto it = map_.find(key);
    if (it == map_.end()) return false;

    items_.splice(items_.begin(), items_, it->second);
    outImage = slint::Image(it->second->buffer);
    return true;
}
slint::Image SoundData::LruImageCache::put(const std::string& key, slint::SharedPixelBuffer<slint::Rgba8Pixel> buffer) {
    const size_t entryBytes = static_cast<size_t>(buffer.width()) * buffer.height() * sizeof(slint::Rgba8Pixel);

    if (const auto it = map_.find(key); it != map_.end()) {
        currentBytes_ -= it->second->bytes;
        items_.erase(it->second);
        map_.erase(it);
    }

    while (!items_.empty() && currentBytes_ + entryBytes > maxBytes_) {
        const auto& last = items_.back();
        currentBytes_ -= last.bytes;
        map_.erase(last.key);
        items_.pop_back();
    }

    items_.push_front(Entry{.key = key, .buffer = std::move(buffer), .bytes = entryBytes});
    map_[key] = items_.begin();
    currentBytes_ += entryBytes;

    return slint::Image(items_.front().buffer);
}

slint::Image CreateNoiseImage(const std::string& assetKey, const uint32_t width = 128, const uint32_t height = 128) {
    static SoundData::LruImageCache noiseCache(2 * 1024 * 1024);

    const std::string fullKey = assetKey + '_' + std::to_string(width) + 'x' + std::to_string(height);

    slint::Image noiseImage;
    if (noiseCache.get(fullKey, noiseImage)) {
        return noiseImage;
    }

    PixelBuffer pixelBuffer(width, height);

    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 255);

    for (auto* pixel = pixelBuffer.begin(); pixel != pixelBuffer.end(); ++pixel) {
        pixel->r = static_cast<uint8_t>(dist(rng));
        pixel->g = static_cast<uint8_t>(dist(rng));
        pixel->b = static_cast<uint8_t>(dist(rng));
        pixel->a = 255;
    }

    return noiseCache.put(fullKey, std::move(pixelBuffer));
}


TagLib::ByteVector getID3v2Image(const TagLib::ID3v2::Tag* tag) {
    if (!tag) return {};
    auto frames = tag->frameListMap()["APIC"];
    if (frames.isEmpty()) return {};

    auto* picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
    return picFrame ? picFrame->picture() : TagLib::ByteVector();
}

std::vector<uint8_t> SoundData::GetAudioCoverBytes(const std::string& assetKey,  const std::filesystem::path& filePath) {
    if (filePath.empty()) return {};

    const TagLib::FileRef f(filePath.c_str());
    if (f.isNull() || !f.file() || !f.file()->isValid()) {
        return {};
    }

    TagLib::ByteVector rawCover;
    TagLib::File* filePtr = f.file();

    std::string ext = filePath.extension().string();
    std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (ext == ".mp3") {
        if (auto* mp3File = dynamic_cast<TagLib::MPEG::File*>(filePtr)) {
            rawCover = getID3v2Image(mp3File->ID3v2Tag());
        }
    }
    else if (ext == ".wav") {
        if (const auto* wavFile = dynamic_cast<TagLib::RIFF::WAV::File*>(filePtr)) {
            rawCover = getID3v2Image(wavFile->ID3v2Tag());
        }
    }
    else if (ext == ".flac") {
        if (auto* flacFile = dynamic_cast<TagLib::FLAC::File*>(filePtr)) {
            if (const auto& picList = flacFile->pictureList(); !picList.isEmpty()) {
                rawCover = picList.front()->data();
            }
            if (rawCover.isEmpty()) {
                rawCover = getID3v2Image(flacFile->ID3v2Tag());
            }
        }
    }
    else if (ext == ".mp4" || ext == ".m4a") {
        if (const auto* mp4File = dynamic_cast<TagLib::MP4::File*>(filePtr)) {
            if (const auto* tag = mp4File->tag()) {
                if (tag->contains("covr")) {
                    const auto coverList = tag->item("covr").toCoverArtList();
                    if (!coverList.isEmpty()) {
                        rawCover = coverList.front().data();
                    }
                }
            }
        }
    }
    if (rawCover.isEmpty()) return {};

    return {
        reinterpret_cast<const uint8_t*>(rawCover.data()),
        reinterpret_cast<const uint8_t*>(rawCover.data()) + rawCover.size()
    };
}

static slint::SharedPixelBuffer<slint::Rgba8Pixel> VImageToSlintBuffer(vips::VImage image) {
    if (image.interpretation() != VIPS_INTERPRETATION_sRGB) {
        image = image.colourspace(VIPS_INTERPRETATION_sRGB);
    }
    if (image.format() != VIPS_FORMAT_UCHAR) {
        image = image.cast(VIPS_FORMAT_UCHAR);
    }
    if (image.bands() == 3) {
        image = image.bandjoin(255);
    }
    else if (image.bands() > 4) {
        image = image.extract_band(0, vips::VImage::option()->set("n", 4));
    }

    const auto w = static_cast<uint32_t>(image.width());
    const auto h = static_cast<uint32_t>(image.height());
    slint::SharedPixelBuffer<slint::Rgba8Pixel> pixel_buffer(w, h);

    size_t memorySize = 0;
    void* rawPixels = image.write_to_memory(&memorySize);
    if (memorySize == w * h * sizeof(slint::Rgba8Pixel)) {
        std::memcpy(pixel_buffer.begin(), rawPixels, memorySize);
    }
    g_free(rawPixels);
    return pixel_buffer;
}

slint::Image SoundData::GetImageFromBytes(const std::string& assetKey, const int maxDimension, const std::filesystem::path& filePath)
{
    static LruImageCache imageCache(16 * 1024 * 1024);

    const std::string fullKey = assetKey + '_' + std::to_string(maxDimension);

    slint::Image cachedImg;
    if (imageCache.get(fullKey, cachedImg)) {
        return cachedImg;
    }

    std::filesystem::path thumbDir = getDataDir(DataType::Cache) / "cache";

    json entry = cacheManager_->getEntry(assetKey);
    if (!entry.is_object()) {
        entry = json::object();
    }
    if (!entry.is_null() && entry.contains("hasCover")) {
        if (!entry["hasCover"].get<bool>()) {
            return CreateNoiseImage(assetKey);
        }

        if (entry.contains("coverHash")) {
            const std::string coverHash = entry["coverHash"].get<std::string>();
            const std::string hashKey = coverHash + '_' + std::to_string(maxDimension);

            if (imageCache.get(hashKey, cachedImg)) {
                return cachedImg;
            }

            const std::filesystem::path thumbPath = thumbDir/ (coverHash + '_' + std::to_string(maxDimension) + ".webp");
            if (std::filesystem::exists(thumbPath)) {
                try {
                    const auto u8Path = thumbPath.u8string();
                    vips::VImage diskImg = vips::VImage::new_from_file(
                        reinterpret_cast<const char*>(u8Path.c_str())
                    );
                    return imageCache.put(fullKey, VImageToSlintBuffer(diskImg));
                } catch (const vips::VError&) {
                    // corrupted thumbnail -> rebuild
                }
            }
        }
    }

    const std::vector<uint8_t>& imageBytes = GetAudioCoverBytes(assetKey, filePath);
    if (imageBytes.empty()) {
        entry["hasCover"] = false;
        cacheManager_->setEntry(assetKey, entry);
        return CreateNoiseImage(assetKey);
    }

    const std::string_view byteView(reinterpret_cast<const char*>(imageBytes.data()), imageBytes.size());
    const std::string coverHash = std::to_string(std::hash<std::string_view>{}(byteView));
    const std::filesystem::path thumbPath = thumbDir / (coverHash + '_' + std::to_string(maxDimension) + ".webp");
    const auto u8Path = thumbPath.u8string();

    try {
        if (std::filesystem::exists(thumbPath)) {
            vips::VImage diskImg = vips::VImage::new_from_file(
                reinterpret_cast<const char*>(u8Path.c_str())
            );
            entry["hasCover"] = true;
            entry["coverHash"] = coverHash;
            cacheManager_->setEntry(assetKey, entry);
            return imageCache.put(fullKey, VImageToSlintBuffer(diskImg));
        }

        vips::VImage image = vips::VImage::new_from_buffer(imageBytes.data(), imageBytes.size(), "")
            .thumbnail_image(
                maxDimension,
                vips::VImage::option()
                    ->set("height", maxDimension)
                    ->set("size", VIPS_SIZE_DOWN)
            );

        std::filesystem::create_directories(thumbDir);
        image = image.copy_memory();
        image.webpsave(
            reinterpret_cast<const char*>(u8Path.c_str()),
            vips::VImage::option()->set("Q", 90)
        );

        entry["hasCover"] = true;
        entry["coverHash"] = coverHash;
        cacheManager_->setEntry(assetKey, entry);

        return imageCache.put(fullKey, VImageToSlintBuffer(image));
    } catch (const vips::VError& err) {
        std::cerr << "Failed to decode image with libvips -> " << err.what() << '\n';
        return CreateNoiseImage(assetKey);
    }
}

std::filesystem::path SoundData::getDataDir(const DataType type) { // 0 = config, 1 = cache
#ifdef _WIN32
    if (type != DataType::Music) {
        wchar_t path[MAX_PATH] = { 0 };
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        return std::filesystem::path(path).parent_path();
    }
    else {
        return configManager_->getConfig().value("text/music-directory", "");
    }
#else
    const char* homeDir = std::getenv("HOME");
    if (homeDir) {
        if (type == DataType::Config) { // config
            std::filesystem::path configPath = std::filesystem::path(homeDir) / ".config" / "OpenPlayer_Config";
            std::filesystem::create_directories(configPath);
            return configPath;
        }
        else if (type == DataType::Cache) { // cache
            std::filesystem::path cachePath = std::filesystem::path(homeDir) / ".cache" / "OpenPlayer_Cache";
            std::filesystem::create_directories(cachePath);
            return cachePath;
        }
        else if (type == DataType::Music) { // music
            return configManager_->getConfig().value("text/music-directory", "");
        }
    }

    return std::filesystem::current_path();
#endif
}

// ColorUtils
slint::SharedString colorToHex(const slint::Color color) {
    char hex[10];
    std::snprintf(hex, sizeof(hex), "#%02x%02x%02x%02x", color.red(), color.green(), color.blue(), color.alpha());
    return {hex};
}

slint::Color hexToColor(std::string_view hex) {
    if (!hex.empty() && hex.front() == '#') {
        hex.remove_prefix(1);
    }

    auto parse_byte = [](const std::string_view pair) -> uint8_t {
        unsigned int val = 0;
        for (const char c : pair) {
            val <<= 4;
            if (c >= '0' && c <= '9') val |= static_cast<uint8_t>(c - '0');
            else if (c >= 'a' && c <= 'f') val |= static_cast<uint8_t>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') val |= static_cast<uint8_t>(c - 'A' + 10);
        }
        return static_cast<uint8_t>(val);
    };

    if (hex.size() == 6) {
        return slint::Color::from_rgb_uint8(
            parse_byte(hex.substr(0, 2)),
            parse_byte(hex.substr(2, 2)),
            parse_byte(hex.substr(4, 2))
        );
    }
    if (hex.size() == 8) {
        return slint::Color::from_argb_uint8(
            parse_byte(hex.substr(6, 2)),
            parse_byte(hex.substr(0, 2)),
            parse_byte(hex.substr(2, 2)),
            parse_byte(hex.substr(4, 2))
        );
    }

    std::cerr << "Invalid color type: " << hex << std::endl;
    return {};
}

Download::TempImage::TempImage(const std::string &url, const int maxDimension) {

    if (url.empty()) {
        image_ = CreateNoiseImage("baseNoiseImg");
    }
    else {
        cpr::Response response = cpr::Get(cpr::Url{url}, cpr::Timeout{std::chrono::seconds{3}});

        if (response.status_code != 200) {
            std::cout << "Failed to get file\n";
            image_ = CreateNoiseImage("baseNoiseImg");
        }
        else {
            try {
                vips::VImage image = vips::VImage::new_from_buffer(response.text.data(), response.text.size(), "")
                    .thumbnail_image(
                        maxDimension,
                        vips::VImage::option()
                            ->set("height", maxDimension)
                            ->set("size", VIPS_SIZE_DOWN)
                    );

                const slint::SharedPixelBuffer<slint::Rgba8Pixel> pixelBuffer = VImageToSlintBuffer(image);

                image_ = slint::Image(pixelBuffer);
            } catch (const vips::VError& e) {
                std::cerr << "VIPS_ERROR -> " << e.what() << '\n';
                image_ = CreateNoiseImage("baseNoiseImg");
            }
        }
    }
}

slint::Image& Download::TempImage::get() {
    return image_;
}
