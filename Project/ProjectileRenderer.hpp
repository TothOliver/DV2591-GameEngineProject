#pragma once
#include "ProjectileManager.hpp"
#include "RaylibHelper.hpp"
#include "raylib.h"
#include <unordered_map>
#include <string>

// Handles rendering projectiles using RaylibHelper for asset loading
class ProjectileRenderer
{
public:
    ProjectileRenderer(RaylibHelper& raylibHelper);
    ~ProjectileRenderer();

    void RenderProjectiles(const ProjectileManager& projectileManager);
    void CleanupUnusedAssets();

private:
    struct ProjectileAsset
    {
        Model model;
        Texture2D texture;
        int refCount = 0;
        bool isLoaded = false;
        std::string modelName;  // For RaylibHelper release
    };

    RaylibHelper* m_raylibHelper;
    std::unordered_map<std::string, ProjectileAsset> m_projectileAssets;

    ProjectileAsset& GetOrLoadAsset(const std::string& meshGUID, const std::string& textureGUID);
};

// Implementation
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
        // Create unique name for this projectile type
        asset.modelName = "projectile_" + key;

        // Load model and texture through RaylibHelper
        asset.model = m_raylibHelper->GetModel(meshGUID, asset.modelName);
        asset.texture = m_raylibHelper->GetTexture(textureGUID);

        // Apply texture to model
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

        // Get or load the asset for this projectile type
        auto& asset = GetOrLoadAsset(proj->GetMeshGUID(), proj->GetTextureGUID());
        asset.refCount++;

        // Render the projectile
        Vector3 position = {
            proj->GetPosX(),
            proj->GetPosY(),
            proj->GetPosZ()
        };

        DrawModel(asset.model, position, 0.3f, WHITE);

        // Optional: Draw velocity vector for debugging
        // Vector3 vel = { proj->GetDirX() * 0.5f, 
        //                 proj->GetDirY() * 0.5f, 
        //                 proj->GetDirZ() * 0.5f };
        // DrawLine3D(position, Vector3Add(position, vel), YELLOW);
    }
}

void ProjectileRenderer::CleanupUnusedAssets()
{
    // Remove assets that aren't being used
    for (auto it = m_projectileAssets.begin(); it != m_projectileAssets.end(); )
    {
        if (it->second.refCount == 0)
        {
            // Release through RaylibHelper
            m_raylibHelper->ReleaseModel(it->second.modelName);
            m_raylibHelper->ReleaseTexture(it->first.substr(0, it->first.find('_')));

            it = m_projectileAssets.erase(it);
        }
        else
        {
            ++it;
        }
    }
}