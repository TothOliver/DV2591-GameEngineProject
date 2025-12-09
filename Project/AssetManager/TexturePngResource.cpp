#include "TexturePngResource.hpp"
#include "raylib.h"


TexturePng::~TexturePng()
{
	Unload();
}

bool TexturePng::Load(const std::vector<uint8_t>& data)
{
	// load le texture
	m_size = data.size();
	Image img = LoadImageFromMemory(".png", data.data(), m_size);

	if (img.data == nullptr)
	{
		std::cerr << "TexturePng: Failed to load the image." << std::endl;
		return false;
	}

	m_imageData = (unsigned char*)img.data;
	m_width = img.width;
	m_height = img.height;
	m_channels = 4;

	return true;
}

bool TexturePng::Unload()
{
	//unload un texture
	if (m_imageData)
	{
		UnloadImage({ m_imageData, m_width, m_height, m_channels });
		m_imageData = nullptr;
		m_height = 0, m_width = 0, m_channels = 0, m_size = 0;
		return true;
	}

	return false;
}

const unsigned char* TexturePng::GetTexture()
{
	return m_imageData;
}
