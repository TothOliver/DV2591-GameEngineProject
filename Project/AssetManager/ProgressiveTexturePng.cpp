#include "AssetManager/ProgressiveTexturePng.hpp"
#include "raylib.h"

std::string ProgressiveTexturePng::GetNextLODGuid() const
{
    if (!HasHigherLOD()) return "";

    // Extract base GUID and append next LOD level
    // Assumes format: "001_lod0", "001_lod1", etc.
    std::string baseGuid = m_guid;
    size_t lodPos = baseGuid.find("_lod");
    if (lodPos != std::string::npos) {
        baseGuid = baseGuid.substr(0, lodPos);
    }

    return baseGuid + "_lod" + std::to_string(m_currentLOD + 1);
}

bool ProgressiveTexturePng::Load(const std::vector<uint8_t>& data)
{
    m_size = data.size();
    Image img = LoadImageFromMemory(".png", data.data(), m_size);
    if (!img.data) return false;

    size_t imgSize = img.width * img.height * 4;
    m_imageData = (unsigned char*)malloc(imgSize); // take ownership safely
    memcpy(m_imageData, img.data, imgSize);

    m_width = img.width;
    m_height = img.height;
    m_channels = 4;

    UnloadImage(img); // now safe

    // detect lod
    size_t lodPos = m_guid.find("_lod");
    m_currentLOD = (lodPos != std::string::npos) ? std::stoi(m_guid.substr(lodPos + 4)) : 0;

    return true;
}


bool ProgressiveTexturePng::LoadHigherLOD(const std::vector<uint8_t>& data, int width, int height)
{
    if (data.empty() || width <= 0 || height <= 0)
        return false;

    const size_t expectedSize = static_cast<size_t>(width) * height * 4;
    if (data.size() < expectedSize)
        return false;

    m_pendingImage.resize(expectedSize);
    memcpy(m_pendingImage.data(), data.data(), expectedSize);

    m_pendingW = width;
    m_pendingH = height;
    m_pendingC = 4;
    m_size = expectedSize;

    return true;
}

void ProgressiveTexturePng::SwapToHigherLOD()
{
    if (m_pendingImage.empty()) return;

    // free old
    if (m_imageData)
        free(m_imageData);

    // replace with pending
    size_t newSize = m_pendingImage.size();
    m_imageData = (unsigned char*)malloc(newSize);
    memcpy(m_imageData, m_pendingImage.data(), newSize);

    m_width = m_pendingW;
    m_height = m_pendingH;
    m_channels = m_pendingC;
    m_size = newSize;

    m_pendingImage.clear();
    m_currentLOD++;
}


bool ProgressiveTexturePng::Unload()
{
    if (m_imageData) {
        free(m_imageData);
        m_imageData = nullptr;
    }
    m_pendingImage.clear();
    m_width = m_height = m_channels = 0;
    m_size = 0;
    return true;
}


const unsigned char* ProgressiveTexturePng::GetTexture()
{
    return m_imageData;
}