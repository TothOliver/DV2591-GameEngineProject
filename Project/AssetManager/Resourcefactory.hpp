#pragma once

#include <memory>
#include <string>
#include "AssetManager/IResource.hpp"

class ResourceFactory
{
public:
    static std::shared_ptr<IResource> Create(const std::string& guid, ResourceType type);
};