#pragma once
#include <iostream>
#include "AssetManager/IResource.hpp"


class TexturePng : public IResource
{
public:
	TexturePng(std::string GUID) : IResource(GUID, ResourceType::TexturePng) {}
	~TexturePng();

	bool Load(const std::vector<uint8_t>& data) override;
	bool Unload() override;

	const unsigned char* GetTexture();
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetChannels() const { return m_channels; }

private:
	int m_width = 0;
	int m_height = 0; 
	int m_channels = 0;

	unsigned char* m_imageData = nullptr;

};