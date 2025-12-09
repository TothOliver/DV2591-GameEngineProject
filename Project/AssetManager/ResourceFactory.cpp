#include "AssetManager/ResourceFactory.hpp"
#include "AssetManager/TexturePngResource.hpp"
#include "AssetManager/MeshObjResource.hpp"
#include "AssetManager/ProgressiveTexturePng.hpp"
#include <iostream>

std::shared_ptr<IResource> ResourceFactory::Create(const std::string& guid, ResourceType type)
{
    switch (type)
    {
    case ResourceType::TexturePng:
        return std::make_shared<TexturePng>(guid);

    case ResourceType::Mesh:
        return std::make_shared<MeshObj>(guid);
        return nullptr;

    case ResourceType::ProgressiveTexturePng:
        return std::make_shared<ProgressiveTexturePng>(guid);
     
    default:
        std::cerr << "ResourceFactory: Unsupported resource type\n";
        return nullptr;
    }
}
