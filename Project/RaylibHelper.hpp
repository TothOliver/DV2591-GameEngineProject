#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>

#include "AssetManager/AssetManager.hpp"
#include "AssetManager/TexturePngResource.hpp"
#include "AssetManager/MeshObjResource.hpp"
#include "AssetManager/ProgressiveTexturePng.hpp"
#include "AssetManager/tinyobjToRaylib.hpp"
#include "raylib.h"

struct TextureEntry
{
	Texture2D texture;
	int refCount = 0;
}; 

struct ModelEntry
{
	Model model;
	std::string guid;
	int refCount = 0;
};

class RaylibHelper
{
public:
	RaylibHelper(AssetManager& assetManager);
	~RaylibHelper();

	Texture2D GetTexture(std::string GUID);
	Model GetModel(std::string GUID, std::string name);

	void ReleaseTexture(std::string GUID);
	void ReleaseModel(std::string name);


	void ForceUnloadTexture(std::string GUID);
	void ForceUnloadModel(std::string name);

	void RequestProgressiveTexture(const std::string& baseGuid, int maxLOD);
	void Update(float dt);
	void CleanUp();

private:
	Texture2D GenerateTexture(std::shared_ptr<IResource>& baseRes);
	Texture2D GenerateBaseTexture();
	Model GenerateBaseModel();

	std::unordered_map<std::string, TextureEntry> m_textures;
	std::unordered_map<std::string, ModelEntry> m_models;

	struct ProgressiveLODState
	{
		std::string baseGuid;
		std::string nextGuid;
		std::string pendingUnload;

		float timer = 0.0f;
		float delay = 2.0f;

		int maxLOD = 0;
		bool active = false;
	};

	std::unordered_map<std::string, ProgressiveLODState> m_progressiveLODs;

	Texture2D m_baseTexture;
	Model m_baseModel;

	AssetManager* m_assetManager;
};
