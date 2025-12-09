#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include "raylib.h"

struct TextureEntry
{
	Texture2D texture;
	int refCount = 0;
}; 

struct ModelEntry
{
	Model model;
	int refCount = 0;
};

class RaylibHelper
{
public:
	RaylibHelper();
	~RaylibHelper();

	Texture2D GetTexture(std::string GUID);
	Model GetModel(std::string GUID);

	void releaseTexture(std::string GUID);
	void releaseModel(std::string GUID);

	void cleanup();

private:
	std::unordered_map<std::string, TextureEntry> m_textures;
	std::unordered_map<std::string, ModelEntry> m_models;

	//potential asset manager in here?
};
