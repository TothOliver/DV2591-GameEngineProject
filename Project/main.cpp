#include "AssetManager.hpp"
#include "PackagingTool.hpp"
#include "TexturePngResource.hpp"
#include "MeshObjResource.hpp"
#include "ProgressiveTexturePng.hpp"
#include "TinyobjToRaylib.hpp"
#include "raylib.h"

#include <iostream>
#include <memory>

void SetTexture(Model& model, std::shared_ptr<IResource>& baseRes)
{
    if (baseRes)
    {
        ResourceType type = baseRes->GetResourceType();

        if (type == ResourceType::TexturePng)
        {
            auto pngRes = std::dynamic_pointer_cast<TexturePng>(baseRes);

            if (!pngRes)
            {
                std::cerr << "Error: Resource for GUID " << baseRes->GetGUID() << " is not a TexturePng.\n";
            }
            else
            {
                Texture2D texture{};
                const unsigned char* pixels = pngRes->GetTexture();

                int width = pngRes->GetWidth();
                int height = pngRes->GetHeight();

                Image img{};
                img.data = (void*)pixels;
                img.width = width;
                img.height = height;
                img.mipmaps = 1;
                img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
                texture = LoadTextureFromImage(img);

                std::cout << "Async texture ready: " << width << "x" << height << "\n";
                model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
            }
        }
        else if (type == ResourceType::ProgressiveTexturePng)
        {
            auto pngRes = std::dynamic_pointer_cast<ProgressiveTexturePng>(baseRes);

            if (!pngRes)
            {
                std::cerr << "Error: Resource for GUID " << baseRes->GetGUID() << " is not a TexturePng.\n";
            }
            else
            {
                Texture2D texture{};
                const unsigned char* pixels = pngRes->GetTexture();

                int width = pngRes->GetWidth();
                int height = pngRes->GetHeight();

                Image img{};
                img.data = (void*)pixels;
                img.width = width;
                img.height = height;
                img.mipmaps = 1;
                img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
                texture = LoadTextureFromImage(img);

                std::cout << "Async texture ready: " << width << "x" << height << "\n";
                model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
            }
        }
    }
}

int main()
{
    //packaging tool
    PackagingTool packagingTool;
    packagingTool.buildPackage("AssetsTextList.txt", "Assets.bundle");

    //Asset manager
    AssetManager am(64 * 1024 * 1024, "Assets.bundle");

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

    //Dynamic model using GUID (Texture isnt set here, it is set when fully loaded)
    auto obj = am.Load("101");
    auto mesh = std::dynamic_pointer_cast<MeshObj>(obj);
    Model dynamicModel = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());
    Model background = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());
    Model box1 = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());
    Model box2 = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());
    Model box3 = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());
    Model box4 = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());
    Model box5 = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());
    Model bigBox = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());

    auto roundObj = am.Load("102");
    auto sphereMesh = std::dynamic_pointer_cast<MeshObj>(roundObj);
    Model sphere = ConvertAttribToModel(sphereMesh->GetAttrib(), sphereMesh->GetShapes());

    //Camera creation (obviously)
    Camera camera = { 0 };
    camera.position = { 0.0f, 0.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Texture2D rayTex{};
    bool rayTextureReady = false;

    while (!WindowShouldClose())
    {
        if (!rayTextureReady)
        {
            std::shared_ptr<IResource> baseRes = am.TryGet("001");
            SetTexture(dynamicModel, baseRes);
            rayTextureReady = true;
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        UpdateCamera(&camera, CAMERA_FREE);
        SetMousePosition(width / 2, height / 2);

        if (IsKeyPressed(KEY_ONE))
        {
            am.LoadAsync("001");
            std::shared_ptr<IResource> myResource = am.TryGet("001");
            SetTexture(box1, myResource);
        }

        if (IsKeyPressed(KEY_TWO))
        {
            am.LoadAsync("002");
            std::shared_ptr<IResource> myResource = am.TryGet("002");
            SetTexture(box2, myResource);
        }

        if (IsKeyPressed(KEY_THREE))
        {
            am.LoadAsync("003");
            std::shared_ptr<IResource> myResource = am.TryGet("003");
            SetTexture(box3, myResource);
            SetTexture(sphere, myResource);
        }

        if (IsKeyPressed(KEY_FOUR))
        {
            //am.LoadAsync("004_lod0");
            std::shared_ptr<IResource> myResource = am.TryGet("004_lod0");
            SetTexture(bigBox, myResource);
        }

        if (IsKeyPressed(KEY_X))
        {
            Texture2D empty{};
            box1.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = empty;
            box2.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = empty;
            box3.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = empty;
            box4.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = empty;
            box5.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = empty;


            am.Unload("001");
            am.Unload("002");
            am.Unload("003");
        }

        //Background
        DrawModel(background, { 0, -22, 0 }, 20.0f, DARKGREEN);
        DrawModel(background, { 40, 0, 0 }, 20.0f, DARKBLUE);
        DrawModel(background, { -40, 0, 0 }, 20.0f, DARKBLUE);
        DrawModel(background, { 0, 0, 40 }, 20.0f, DARKBLUE);
        DrawModel(background, { 0, 0, -40 }, 20.0f, LIGHTGRAY);
        DrawModel(background, { 0, 30, 0 }, 20.0f, DARKBLUE);

        //small boxes
        DrawModel(dynamicModel, { -3, 3, -3 }, 1.0f, LIGHTGRAY);
        DrawModel(box1, { 0, 3, -3 }, 1.0f, WHITE);
        DrawModel(box2, { 3, 3, -3 }, 1.0f, WHITE);
        DrawModel(box3, { -3, 0, -3 }, 1.0f, WHITE);
        DrawModel(box4, { 0, 0, -3 }, 1.0f, WHITE);
        DrawModel(box5, { 3, 0, -3 }, 1.0f, WHITE);

        //Progressive
        DrawModel(bigBox, { -12, 1, -6 }, 2.0f, WHITE);

        //test
        DrawModel(sphere, { 10, 1, -3 }, 2.0f, WHITE);

        EndMode3D();

        //2D
        DrawRectangle(20, 20, 330, 100, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(20, 20, 330, 100, BLUE);

        DrawText("Controls:", 30, 30, 30, BLACK);
        DrawText("1-3 -> Load textures", 30, 60, 30, BLACK);
        DrawText("X -> Clean all", 30, 90, 30, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
