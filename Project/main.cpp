#include "AssetManager/PackagingTool.hpp"
#include "RaylibHelper.hpp"
#include "ProjectileManager.hpp"
#include "ProjectileRenderer.hpp"
#include "MemoryManager/StackAllocator.hpp"
#include "ExplosionSystem.hpp"
#include "raymath.h"
#include "raylib.h"
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>

struct MemoryDebugInfo 
{
    size_t stackUsedBytes = 0;
    size_t stackCapacityBytes = 0;
    float stackUsageRatio = 0.0f;
};

struct AssetDebugInfo
{
    size_t memoryUsedBytes = 0;
    size_t memoryLimitBytes = 0;
    size_t loadedResourceCount = 0;
    size_t asyncJobsInFlight = 0;
    size_t totalEvictions = 0;
};

struct PendingTextureSet
{
    std::string guid;
    Model* model = nullptr;
    bool applied = false;
};

struct PendingModelSet
{
    std::string guid;
    std::string name;
    Model* outModel = nullptr;
    bool applied = false;
};

void SetTexture(Model& model, Texture2D& texture)
{
    if (model.materials == nullptr || model.materialCount <= 0)
        return;

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
}

void DrawStackAllocatorOverlay(const MemoryDebugInfo& info)
{
    const float margin = 10.0f;
    const float panelW = 260.0f;
    const float panelH = 60.0f;

    const float panelX = GetScreenWidth() - panelW - margin;
    const float panelY = margin;

    DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.6f));
    DrawRectangleLines(panelX, panelY, panelW, panelH, RAYWHITE);

    DrawText("Stack Allocator", panelX + 10, panelY + 5, 16, RAYWHITE);

    float usedKB = info.stackUsedBytes / 1024.0f;
    float capKB = info.stackCapacityBytes / 1024.0f;

    char buffer[128];
    std::snprintf(buffer, sizeof(buffer),
        "Used: %.1f / %.1f KB", usedKB, capKB);
    DrawText(buffer, panelX + 10, panelY + 25, 14, RAYWHITE);

    float barX = panelX + 10;
    float barY = panelY + 40;
    float barW = panelW - 20;
    float barH = 10;

    float ratio = info.stackUsageRatio;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    DrawRectangle(barX, barY, barW, barH, DARKGRAY);
    DrawRectangle(barX, barY, barW * ratio, barH, GREEN);
}

void DrawAssetManagerOverlay(const AssetDebugInfo info)
{
    const float margin = 10.0f;
    const float panelW = 260.0f;
    const float panelH = 90.0f;

    // Place it just BELOW the stack allocator panel
    const float panelX = GetScreenWidth() - panelW - margin;
    const float panelY = margin + 60.0f + 10.0f; // stack panel height + small gap

    DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.6f));
    DrawRectangleLines(panelX, panelY, panelW, panelH, RAYWHITE);

    DrawText("Resource Manager", panelX + 10, panelY + 5, 16, RAYWHITE);

    // Memory usage
    float usedMB = info.memoryUsedBytes / (1024.0f * 1024.0f);
    float capMB = info.memoryLimitBytes / (1024.0f * 1024.0f);

    char buffer[128];
    std::snprintf(buffer, sizeof(buffer),
        "Mem: %.1f / %.1f MB", usedMB, capMB);
    DrawText(buffer, panelX + 10, panelY + 25, 14, RAYWHITE);

    // Memory bar
    const float barX = panelX + 10;
    const float barY = panelY + 40;
    const float barW = panelW - 20;
    const float barH = 10;

    float ratio = 0.0f;
    if (info.memoryLimitBytes > 0)
    {
        ratio = static_cast<float>(info.memoryUsedBytes) /
            static_cast<float>(info.memoryLimitBytes);
    }
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    DrawRectangle(barX, barY, barW, barH, DARKGRAY);
    DrawRectangle(barX, barY, barW * ratio, barH, GREEN);

    // Loaded resources and async jobs
    std::snprintf(buffer, sizeof(buffer),
        "Loaded: %zu | Jobs: %zu",
        info.loadedResourceCount,
        info.asyncJobsInFlight);
    DrawText(buffer, panelX + 10, panelY + 55, 14, RAYWHITE);

    // Optional evictions line
    std::snprintf(buffer, sizeof(buffer),
        "Evictions: %zu",
        info.totalEvictions);
    DrawText(buffer, panelX + 10, panelY + 70, 14, RAYWHITE);
}

int main()
{
    //packaging tool
    PackagingTool packagingTool;
    packagingTool.buildPackage("AssetsListNew.txt", "Assets.bundle");

    //Asset manager
    AssetManager am(64 * 1024 * 1024, "Assets.bundle");
    AssetDebugInfo g_assetsDebug;

    StackAllocator frameAllocator(64 * 1024);  
    ExplosionSystem explosionSystem(frameAllocator);
    MemoryDebugInfo g_memoryDebug;

    int width = 1280;
    int height = 720;
    InitWindow(width, height, "Tony Rickardsson");
    SetTargetFPS(60);
    DisableCursor();

    //Raylib helper
    RaylibHelper rh(am);
    
    //Dynamic model using GUID (Texture isnt set here, it is set when fully loaded)
    am.Load("cube");
    Model background = rh.GetModel("cube", "background");
    am.Load("sphere");
    Model sphere = rh.GetModel("sphere", "sphere");
    Model snowman = rh.GetModel("snowman", "snowman");
    Model snowpile = rh.GetModel("snowpile", "snowpile");
    Model snowflat = rh.GetModel("snowflat", "snowflat");
    Model tree1 = rh.GetModel("tree", "tree1");
    Model tree2 = rh.GetModel("treeA", "tree2");
    Model tree3 = rh.GetModel("treeB", "tree3");
    bool isLoaded1 = false;
    bool isLoaded2 = false;
    bool isLoaded3 = false;

    //progressive stuff
    std::string LODName;
    bool higherLODRequested = false;
    float LODTimer = 0.0;

    //Camera creation (obviously)
    Camera camera = { 0 };
    camera.position = { 0.0f, 0.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    //PROJECTILE SYSTEM
    ProjectileManager projectileManager;
    projectileManager.Initialize(1000);

    ProjectileRenderer projectileRenderer(rh);

    std::string currentProjectileMesh = "sphere";
    std::string currentProjectileTexture = "001";

    float shootCooldown = 0.0f;

    //STRESS loader
    static bool stressOn = false;
    static float stressTimer = 0.0f;
    static int stressLoadsPerTick = 20;
    static float stressTickRate = 0.10f;

    static int guidMin = 100;
    static int guidMax = 200;
    static int guidCursor = guidMin;

    auto NextGuid = [&]()
    {
        std::string g = std::to_string(guidCursor++);
        if (guidCursor > guidMax) guidCursor = guidMin;
        return g;
    };


    std::unordered_map<std::string, PendingTextureSet> pendingByName;
    std::unordered_set<std::string> pendingGuids;
    std::unordered_map<std::string, PendingModelSet> pendingModelsByName;
    std::unordered_set<std::string> pendingModelGuids;

    auto RequestTextureFor = [&](const std::string& name, Model& model, const std::string& guid)
    {
        pendingByName[name] = PendingTextureSet{ guid, &model, false };
        am.LoadAsync(guid);
        pendingGuids.insert(guid);
    };

    auto ResolvePendingTextures = [&]()
    {
        for (auto it = pendingGuids.begin(); it != pendingGuids.end(); )
        {
            const std::string& guid = *it;

            if (am.TryGet(guid) == nullptr)
            {
                ++it;
                continue;
            }

            for (auto& [name, pending] : pendingByName)
            {
                if (!pending.applied && pending.guid == guid && pending.model)
                {
                    Texture2D tex = rh.GetTexture(guid);
                    SetTexture(*pending.model, tex);
                    pending.applied = true;
                }
            }

            it = pendingGuids.erase(it);
        }
    };

    auto RequestModelFor = [&](const std::string& name, Model& model, const std::string& guid)
    {
        pendingModelsByName[name] = PendingModelSet{ guid, name, &model, false };
        am.LoadAsync(guid);
        pendingModelGuids.insert(guid);
    };

    auto ResolvePendingModels = [&]()
    {
        for (auto it = pendingModelGuids.begin(); it != pendingModelGuids.end(); )
        {
            const std::string& guid = *it;

            if (am.TryGet(guid) == nullptr) { ++it; continue; } // not ready

            for (auto& [name, pending] : pendingModelsByName)
            {
                if (!pending.applied && pending.guid == guid && pending.outModel)
                {
                    *pending.outModel = rh.GetModel(pending.guid, pending.name);
                    pending.applied = true;
                }
            }

            it = pendingModelGuids.erase(it);
        }
    };

    RequestModelFor("snowman", snowman, "snowman");
    RequestModelFor("snowpile", snowpile, "snowpile");
    RequestModelFor("snowflat", snowflat, "snowflat");
    RequestModelFor("tree1", tree1, "tree");
    RequestModelFor("tree2", tree2, "treeA");
    RequestModelFor("tree3", tree3, "treeB");

    RequestTextureFor("snowman", snowman, "colormap");
    
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        shootCooldown -= dt;

        ResolvePendingModels();
        ResolvePendingTextures();

        frameAllocator.Reset();
        explosionSystem.Update(dt);


        if (IsKeyPressed(KEY_T))
        {
            for (int i = 0; i < 3; ++i)
            {
                Vector3 pos = { i * 2.0f, 0.0f, 10.0f };
                explosionSystem.AddExplosion(pos, 4.0f, 1.0f);
            }
        }

        explosionSystem.BuildRendererData();
        AssetManagerDebugInfo amInfo{};
        am.GetDebugInfo(amInfo);

        g_memoryDebug.stackUsedBytes = frameAllocator.GetUsed();
        g_memoryDebug.stackCapacityBytes = frameAllocator.GetCapacity();
        g_memoryDebug.stackUsageRatio = frameAllocator.GetUsageRatio();

        g_assetsDebug.memoryLimitBytes = amInfo.memoryLimit;
        g_assetsDebug.memoryUsedBytes = amInfo.memoryUsed;
        g_assetsDebug.loadedResourceCount = amInfo.loadedResourceCount;
        g_assetsDebug.asyncJobsInFlight = amInfo.asyncQueuedJobs + amInfo.asyncActiveJobs;
        g_assetsDebug.totalEvictions = amInfo.totalEvictions;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        UpdateCamera(&camera, CAMERA_FREE);
        SetMousePosition(width / 2, height / 2);

        if (IsKeyPressed(KEY_ONE))
        {
            RequestModelFor("tree1", tree1, "tree");
            RequestTextureFor("tree1", tree1, "colormap");
            isLoaded1 = true;
        }
        if (IsKeyPressed(KEY_TWO))
        {
            RequestModelFor("tree2", tree2, "treeA");
            RequestTextureFor("tree2", tree2, "colormap");
            isLoaded2 = true;
        }
        if (IsKeyPressed(KEY_THREE))
        {
            RequestModelFor("tree3", tree3, "treeB");
            RequestTextureFor("tree3", tree3, "colormap");
            isLoaded3 = true;
        }
        if (IsKeyPressed(KEY_FOUR))
        {
            RequestTextureFor("snowman", snowman, "colormap");
        }

        if (IsKeyPressed(KEY_X))
        {
            rh.ReleaseModel("tree");
            rh.ReleaseModel("treeA");
            rh.ReleaseModel("treeB");
            rh.ReleaseTexture("colormap");
            rh.ReleaseTexture("colormap");
            rh.ReleaseTexture("colormap");
            rh.ReleaseTexture("colormap");
            isLoaded1 = isLoaded2 = isLoaded3 = false;
            
            //for (int i = 100; i < 200; i++) {
            //    std::string id = i;
            //    rh.ReleaseTexture(id);
            //}


        }

        if (IsKeyPressed(KEY_FIVE))
        {
            stressOn = !stressOn;
        }
        if (stressOn)
        {
            stressTimer += dt;
            if (stressTimer >= stressTickRate)
            {
                stressTimer = 0.0f;

                for (int i = 0; i < stressLoadsPerTick; ++i)
                {
                    am.LoadAsync(NextGuid());
                }
            }
        }


        //PROJECTILES
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && shootCooldown <= 0.0f)
        {
            Vector3 forward = Vector3Subtract(camera.target, camera.position);
            forward = Vector3Normalize(forward);

            Vector3 spawnPos = Vector3Add(camera.position, Vector3Scale(forward, 2.0f));

            projectileManager.Create(
                spawnPos.x, spawnPos.y, spawnPos.z,
                forward.x, forward.y, forward.z,
                20.0f,                              // speed
                5.0f,                               // lifetime
                currentProjectileMesh,              // GUID from asset manager
                currentProjectileTexture            // GUID from asset manager
            );

            shootCooldown = 0.1f;
        }
        // Change projectile type with number keys
        if (IsKeyPressed(KEY_SIX))
        {
            currentProjectileTexture = "sphere";
        }
        if (IsKeyPressed(KEY_SEVEN))
        {
            currentProjectileTexture = "002";
        }
        if (IsKeyPressed(KEY_EIGHT))
        {
            currentProjectileTexture = "003";
        }

        // Update projectiles
        projectileManager.Update(dt);

        //Background
        DrawModel(background, { 0, -22, 0 }, 20.0f, DARKGREEN);
        DrawModel(background, { 40, 0, 0 }, 20.0f, DARKBLUE);
        DrawModel(background, { -40, 0, 0 }, 20.0f, DARKBLUE);
        DrawModel(background, { 0, 0, 40 }, 20.0f, DARKBLUE);
        DrawModel(background, { 0, 0, -40 }, 20.0f, LIGHTGRAY);
        DrawModel(background, { 0, 30, 0 }, 20.0f, DARKBLUE);

        DrawModel(snowman, { 0,-2, 0 }, 1.0f, WHITE);
        DrawModel(snowpile, { 1, -2, 0 }, 1.0f, WHITE);
        DrawModel(snowflat, { 0,-2, 0 }, 1.0f, WHITE);

        if(isLoaded1)
            DrawModel(tree1, { -2,-2, 5 }, 1.0f, WHITE);
        if (isLoaded2)
            DrawModel(tree2, { 0,-2, 5 }, 1.0f, WHITE);
        if (isLoaded3)
            DrawModel(tree3, { 2,-2, 5 }, 1.0f, WHITE);


        //RENDER PROJECTILES
        projectileRenderer.RenderProjectiles(projectileManager);

        // ---- Render explosions (simple spheres) ----
        const ExplosionVertex* verts = explosionSystem.GetVertices();
        size_t vcount = explosionSystem.GetVertexCount();

        for (size_t i = 0; i < vcount; i++)
        {
            const ExplosionVertex& v = verts[i];
            DrawSphere(v.position, v.size, v.color);
        }


        EndMode3D();

        //2D
        DrawRectangle(20, 20, 330, 170, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(20, 20, 330, 170, BLUE);

        DrawText("Controls:", 30, 30, 30, BLACK);
        DrawText("1-3 -> Load textures", 30, 60, 30, BLACK);
        DrawText("4 -> Multiresolution", 30, 90, 30, BLACK);
        DrawText("5 -> Load much", 30, 120, 30, BLACK);
        DrawText("x -> Clean all", 30, 150, 30, BLACK);
        DrawText("T -> EXPLOSION", 30, 180, 30, BLACK);

        DrawStackAllocatorOverlay(g_memoryDebug);
        DrawAssetManagerOverlay(g_assetsDebug);

        EndDrawing();
    }
    
    projectileManager.Shutdown();
    CloseWindow();
    return 0;
}
