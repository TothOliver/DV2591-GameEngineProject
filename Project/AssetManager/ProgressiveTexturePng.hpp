
#pragma once
#include "IResource.hpp"
#include <vector>
#include <string>

class ProgressiveTexturePng : public IResource
{
public:
    ProgressiveTexturePng(const std::string& guid) : IResource(guid, ResourceType::ProgressiveTexturePng) {}
    ~ProgressiveTexturePng() override { Unload(); }

    bool Load(const std::vector<uint8_t>& data) override;
    bool Unload() override;

    // Progressive loading
    bool LoadHigherLOD(const std::vector<uint8_t>& data, int width, int height);
    void SwapToHigherLOD();

    bool HasPendingLOD() const { return !m_pendingImage.empty(); }
    bool TryUpgrade() 
    {
        if (!HasPendingLOD()) return false;
        SwapToHigherLOD();
        return true;
    }

    int GetCurrentLOD() const { return m_currentLOD; }
    int GetMaxLOD() const { return m_maxLOD; }
    bool HasHigherLOD() const { return m_currentLOD < m_maxLOD; }
    std::string GetNextLODGuid() const;

    void SetLODInfo(int maxLOD) { m_maxLOD = maxLOD; }

    // Getters
    unsigned char* GetImageData() const { return m_imageData; }
    const unsigned char* GetTexture();
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannels() const { return m_channels; }

private:
    unsigned char* m_imageData = nullptr;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    size_t m_size = 0;

    // Pending higher LOD
    std::vector<unsigned char> m_pendingImage;
    int m_pendingW = 0;
    int m_pendingH = 0;
    int m_pendingC = 0;

    // NEW: LOD tracking
    int m_currentLOD = 0;  // 0 = lowest res, higher = better quality
    int m_maxLOD = 2;      // Maximum available LOD
};