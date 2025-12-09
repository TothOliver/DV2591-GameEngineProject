#pragma once
#include <string>
#include <vector>
#include "ResourceTypeEnum.h"

class IResource {
public:
    IResource(std::string guid, ResourceType type)
        : m_guid(std::move(guid)), m_type(type) {}

    virtual ~IResource() = default;

    const std::string& GetGUID() const { return m_guid; }
    ResourceType GetResourceType() const { return m_type; }
    size_t GetSize() const { return m_size; }
    bool IsLoaded() const { return m_loaded; }

    // Load from memory buffer
    virtual bool Load(const std::vector<uint8_t>& data) = 0;
    
    // Unload frees internal memory
    virtual bool Unload() = 0;

protected:
    std::string   m_guid;
    ResourceType  m_type;
    size_t        m_size = 0;
    bool          m_loaded = false;
};
