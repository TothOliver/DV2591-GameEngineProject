#include "AssetManager/PackagingTool.hpp"
#include "RaylibHelper.hpp"
#include "ProjectileManager.hpp"
#include "ProjectileRenderer.hpp"
#include "raymath.h"
#include "raylib.h"
#include "MemoryManager/StackAllocator.hpp"
#include "ExplosionSystem.hpp"
#include <iostream>
#include <string>
#include <memory>

struct MemoryDebugInfo 
{
    size_t stackUsedBytes = 0;
    size_t stackCapacityBytes = 0;
    float stackUsageRatio = 0.0f;
};

void SetTexture(Model& model, Texture2D& texture)
{
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

int main()
{
    //packaging tool
    PackagingTool packagingTool;
    packagingTool.buildPackage("AssetsTextList.txt", "Assets.bundle");

    //Asset manager
    AssetManager am(64 * 1024 * 1024, "Assets.bundle");

    StackAllocator frameAllocator(64 * 1024);  
    ExplosionSystem explosionSystem(frameAllocator);
    MemoryDebugInfo g_memoryDebug;

    //Start loading some stuff
    am.LoadAsync("001");
    am.LoadAsync("002");
    am.LoadAsync("003");
    am.LoadAsync("005");
    am.LoadAsync("004_lod0");
    am.LoadAsync("004_lod1");
    am.LoadAsync("004_lod2");

    int width = 1280;
    int height = 720;
    InitWindow(width, height, "Tony Rickardsson");
    SetTargetFPS(60);

    //Raylib helper
    RaylibHelper rh(am);
    
    //Dynamic model using GUID (Texture isnt set here, it is set when fully loaded)
    am.Load("101");
    Model dynamicModel = rh.GetModel("101", "dynamicModel");
    Model background = rh.GetModel("101", "background");
    Model box1 = rh.GetModel("101", "box1");
    Model box2 = rh.GetModel("101", "box2");
    Model box3 = rh.GetModel("101", "box3");
    Model box4 = rh.GetModel("101", "box4");
    Model box5 = rh.GetModel("101", "box5");
    Model bigBox = rh.GetModel("101", "bigBox");

    am.Load("102");
    Model sphere = rh.GetModel("102", "sphere");

    //Fun models :)
    am.Load("103");
    Model tree = rh.GetModel("103", "tree");
    am.Load("104");
    Model table = rh.GetModel("104", "table");
    am.Load("105");
    Model figures = rh.GetModel("105", "figures");

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

    std::string currentProjectileMesh = "102";
    std::string currentProjectileTexture = "003";

    float shootCooldown = 0.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        shootCooldown -= dt;

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

        g_memoryDebug.stackUsedBytes = frameAllocator.GetUsed();
        g_memoryDebug.stackCapacityBytes = frameAllocator.GetCapacity();
        g_memoryDebug.stackUsageRatio = frameAllocator.GetUsageRatio();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        UpdateCamera(&camera, CAMERA_FREE);
        SetMousePosition(width / 2, height / 2);

        if (IsKeyPressed(KEY_ONE))
        {
            Texture2D toe = rh.GetTexture("001");
            SetTexture(box1, toe);
            am.LoadAsync("001");
            std::shared_ptr<IResource> myResource = am.TryGet("001");
            Texture2D texture = rh.GetTexture("001");

        }

        if (IsKeyPressed(KEY_TWO))
        {
            Texture2D hatley = rh.GetTexture("002");
            SetTexture(box2, hatley);
        }

        if (IsKeyPressed(KEY_THREE))
        {
            Texture2D noise = rh.GetTexture("003");
            SetTexture(box3, noise);
            SetTexture(box4, noise);
            SetTexture(box5, noise);
            SetTexture(sphere, noise);
        }

        /*if (IsKeyPressed(KEY_FOUR))
        {
            am.LoadAsync("004_lod0");
            std::shared_ptr<IResource> myResource = am.TryGet("004_lod0");

            if (myResource) {
                SetTexture(bigBox, myResource);

                auto progRes = std::dynamic_pointer_cast<ProgressiveTexturePng>(myResource);
                if (progRes) {
                    progRes->SetLODInfo(2);
                    LODName = progRes->GetNextLODGuid();
                    am.LoadAsync(LODName);
                    higherLODRequested = true;
                    LODTimer = 0.0f;
                }
            }
        }*/

        if (IsKeyPressed(KEY_FIVE))
        {
            for (int i = 200; i < 220; i++)
            {
                am.LoadAsync(std::to_string(i));
            }
        }

        /*if (higherLODRequested)
        {
            LODTimer += GetFrameTime();

            if (LODTimer >= 2.0f)
            {
                auto higherLODRes = am.TryGet(LODName);

                if (higherLODRes) {
                    auto currentRes = am.TryGet("004_lod0");
                    auto currentTex = std::dynamic_pointer_cast<ProgressiveTexturePng>(currentRes);
                    auto higherTex = std::dynamic_pointer_cast<ProgressiveTexturePng>(higherLODRes);

                    if (currentTex && higherTex)
                    {
                        bool loaded = currentTex->LoadHigherLOD(
                            std::vector<uint8_t>(higherTex->GetImageData(),
                                higherTex->GetImageData() + (higherTex->GetWidth() * higherTex->GetHeight() * 4)),
                            higherTex->GetWidth(),
                            higherTex->GetHeight()
                        );

                        if (loaded)
                        {
                            currentTex->TryUpgrade();
                            SetTexture(bigBox, currentRes);

                            am.Unload(LODName);

                            if (currentTex->HasHigherLOD())
                            {
                                LODName = currentTex->GetNextLODGuid();
                                am.LoadAsync(LODName);
                                LODTimer = 0.0f;
                            }
                            else
                            {
                                higherLODRequested = false;
                            }
                        }
                    }
                }
            }
        }*/



        if (IsKeyPressed(KEY_X))
        {
            rh.ForceUnloadTexture("001");
            rh.ForceUnloadTexture("002");
            rh.ForceUnloadTexture("003");
            rh.ForceUnloadTexture("004_lod0");
        }

        //PROJECTILES
        if (IsKeyPressed(KEY_NINE)  && shootCooldown <= 0.0f)
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
            currentProjectileTexture = "001";
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

        //small boxes
        DrawModel(dynamicModel, { -3, 3, -3 }, 1.0f, DARKGRAY);
        DrawModel(box1, { 0, 3, -3 }, 1.0f, WHITE);
        DrawModel(box2, { 3, 3, -3 }, 1.0f, WHITE);
        DrawModel(box3, { -3, 0, -3 }, 1.0f, WHITE);
        DrawModel(box4, { 0, 0, -3 }, 1.0f, WHITE);
        DrawModel(box5, { 3, 0, -3 }, 1.0f, WHITE);

        //Progressive
        DrawModel(bigBox, { -12, 1, -6 }, 2.0f, WHITE);

        //test
        DrawModel(sphere, { 10, 1, -3 }, 2.0f, WHITE);
        DrawModel(tree, { 10, -2, -10 }, 0.5f, LIME);
        DrawModel(table, { -10, -1, -15 }, 0.02f, DARKBROWN);
        DrawModel(figures, { 15, -2, 15 }, 0.15f, RED);
            

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

        EndDrawing();
    }
    projectileManager.Shutdown();
    CloseWindow();
    return 0;
}
