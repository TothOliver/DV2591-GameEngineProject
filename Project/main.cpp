#include "AssetManager/PackagingTool.hpp"
//#include "AssetManager/TinyobjToRaylib.hpp"
#include "RaylibHelper.hpp"
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <string>
#include <memory>
#include "ProjectileManager.hpp"
#include "ProjectileRenderer.hpp"

void SetTexture(Model& model, Texture2D& texture)
{
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
}

int main()
{
    //packaging tool
    PackagingTool packagingTool;
    packagingTool.buildPackage("AssetsTextList.txt", "Assets.bundle");

    //Asset manager
    AssetManager am(10 * 1024 * 1024, "Assets.bundle");

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

    //progressive stuff

    std::string LODName;
    bool higherLODRequested = false;

    //Camera creation (obviously)
    Camera camera = { 0 };
    camera.position = { 0.0f, 0.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float LODTimer = 0.0;

    while (!WindowShouldClose())
    {
        /*if (!rayTextureReady)
        {
            std::shared_ptr<IResource> baseRes = am.TryGet("001");
            SetTexture(dynamicModel, baseRes);
            rayTextureReady = true;
        }*/

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        UpdateCamera(&camera, CAMERA_FREE);
        SetMousePosition(width / 2, height / 2);


        //PROJECTILE SYSTEM
        ProjectileManager projectileManager;
        projectileManager.Initialize(1000);

        ProjectileRenderer projectileRenderer(rh);

        std::string currentProjectileMesh = "102";
        std::string currentProjectileTexture = "003";

        //progressive stuff
        std::string LODName;
        bool higherLODRequested = false;

        //Camera creation
        Camera camera = { 0 };
        camera.position = { 0.0f, 0.0f, 10.0f };
        camera.target = { 0.0f, 0.0f, 0.0f };
        camera.up = { 0.0f, 1.0f, 0.0f };
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        float LODTimer = 0.0;
        float shootCooldown = 0.0f;

        while (!WindowShouldClose())
        {
            float dt = GetFrameTime();
            shootCooldown -= dt;

            BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            UpdateCamera(&camera, CAMERA_FREE);
            SetMousePosition(width / 2, height / 2);

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

            if (IsKeyPressed(KEY_ONE))
            {
                am.LoadAsync("001");
                Texture2D toe = rh.GetTexture("001");
                SetTexture(box1, toe);
            }

            if (IsKeyPressed(KEY_TWO))
            {
                am.LoadAsync("002");
                Texture2D hatley = rh.GetTexture("002");
                SetTexture(box2, hatley);
            }

            if (IsKeyPressed(KEY_THREE))
            {
                am.LoadAsync("003");
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
                rh.ReleaseTexture("001");
                rh.ReleaseTexture("002");
                rh.ReleaseTexture("003");
                rh.ReleaseTexture("004_lod0");
            }

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

            //RENDER PROJECTILES

            projectileRenderer.RenderProjectiles(projectileManager);

            EndMode3D();

            //2D
            DrawRectangle(20, 20, 330, 170, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(20, 20, 330, 170, BLUE);

            DrawText("Controls:", 30, 30, 30, BLACK);
            DrawText("1-3 -> Load textures", 30, 60, 30, BLACK);
            DrawText("4 -> Multiresolution", 30, 90, 30, BLACK);
            DrawText("5 -> Load much", 30, 120, 30, BLACK);
            DrawText("x -> Clean all", 30, 150, 30, BLACK);

            EndDrawing();
        }
        projectileManager.Shutdown();
        CloseWindow();
        return 0;
    }
}
