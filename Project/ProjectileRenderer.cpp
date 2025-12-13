#include "ProjectileRenderer.hpp"

ProjectileRenderer::ProjectileRenderer(RaylibHelper& raylibHelper)
    : m_raylibHelper(&raylibHelper)
{
}

ProjectileRenderer::~ProjectileRenderer()
{
    CleanupUnusedAssets();
}

ProjectileRenderer::ProjectileAsset& ProjectileRenderer::GetOrLoadAsset(
    const std::string& meshGUID, const std::string& textureGUID)
{
    std::string key = meshGUID + "_" + textureGUID;
    auto& asset = m_projectileAssets[key];

    if (!asset.isLoaded)
    {
        asset.modelName = "projectile_" + key;

        asset.model = m_raylibHelper->GetModel(meshGUID, asset.modelName);
        asset.texture = m_raylibHelper->GetTexture(textureGUID);

        if (asset.model.materials != nullptr && asset.model.materialCount > 0)
        {
            asset.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = asset.texture;
        }

        asset.isLoaded = true;
    }

    return asset;
}

void ProjectileRenderer::RenderProjectiles(const ProjectileManager& projectileManager)
{
    // Reset ref counts
    for (auto& pair : m_projectileAssets)
    {
        pair.second.refCount = 0;
    }

    // Render all projectiles
    const auto& projectiles = projectileManager.GetProjectiles();
    for (const Projectile* proj : projectiles)
    {
        if (!proj->IsAlive()) continue;

        auto& asset = GetOrLoadAsset(proj->GetMeshGUID(), proj->GetTextureGUID());
        asset.refCount++;

        Vector3 position = {
            proj->GetPosX(),
            proj->GetPosY(),
            proj->GetPosZ()
        };

        DrawModel(asset.model, position, 0.3f, WHITE);
    }
}

void ProjectileRenderer::CleanupUnusedAssets()
{
    for (auto it = m_projectileAssets.begin(); it != m_projectileAssets.end(); )
    {
        if (it->second.refCount == 0)
        {
            m_raylibHelper->ReleaseModel(it->second.modelName);
            const size_t sep = it->first.find('_');

            const std::string textureGuid = (sep == std::string::npos) ? it->first : it->first.substr(sep + 1);

            m_raylibHelper->ReleaseTexture(textureGuid);

            it = m_projectileAssets.erase(it);
        }
        else
        {
            ++it;
        }
    }
}