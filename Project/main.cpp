#include "AssetManager/PackagingTool.hpp"
//#include "AssetManager/TinyobjToRaylib.hpp"
#include "RaylibHelper.hpp"
#include "raylib.h"

#include "StackAllocator.hpp"
#include "ExplosionSystem.hpp"

#include <iostream>
#include <string>
#include <memory>

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

    StackAllocator frameAllocator(1024 * 1024);  
    ExplosionSystem explosionSystem(frameAllocator);

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
    auto obj = am.Load("101");
    Model dynamicModel = rh.GetModel("101", "dynamicModel");
    Model background = rh.GetModel("101", "background");
    Model box1 = rh.GetModel("101", "box1");
    am.Load("001");
    Texture2D toe = rh.GetTexture("001");
    SetTexture(box1, toe);
  
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
        float dt = GetFrameTime();

        frameAllocator.Reset();
        explosionSystem.Update(dt);

        if (IsKeyPressed(KEY_SPACE))
        {
            Vector3 explosionPos = { 0, 0, 0 };
            explosionSystem.AddExplosion(explosionPos, 4.0f, 1.0f);
        }

        explosionSystem.BuildRendererData();


        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        UpdateCamera(&camera, CAMERA_FREE);
        SetMousePosition(width / 2, height / 2);

        if (IsKeyPressed(KEY_ONE))
        {
            am.LoadAsync("001");
            std::shared_ptr<IResource> myResource = am.TryGet("001");
            //Model model = rh.GetModel("101");
            Texture2D texture = rh.GetTexture("001");
            //SetTexture(model, texture);
        }


        //Background
        DrawModel(background, { 0, -22, 0 }, 20.0f, DARKGREEN);
        DrawModel(background, {40, 0, 0}, 20.0f, DARKBLUE);
        DrawModel(background, {-40, 0, 0}, 20.0f, DARKBLUE);
        DrawModel(background, { 0, 0, 40 }, 20.0f, DARKBLUE);
        DrawModel(background, { 0, 0, -40 }, 20.0f, LIGHTGRAY);
        DrawModel(background, { 0, 30, 0 }, 20.0f, DARKBLUE);

        //small boxes
        DrawModel(dynamicModel, { -3, 3, -3 }, 1.0f, DARKGRAY);
        DrawModel(box1, { 0, 3, -3 }, 1.0f, WHITE);

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

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
