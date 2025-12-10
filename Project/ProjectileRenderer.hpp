#pragma once
#include "ProjectileManager.hpp"
#include "RaylibHelper.hpp"
#include "raylib.h"
#include <unordered_map>
#include <string>

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
        std::string modelName;
    };

    RaylibHelper* m_raylibHelper;
    std::unordered_map<std::string, ProjectileAsset> m_projectileAssets;

    ProjectileAsset& GetOrLoadAsset(const std::string& meshGUID, const std::string& textureGUID);
};

