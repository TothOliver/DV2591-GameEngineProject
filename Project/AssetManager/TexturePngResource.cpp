#include "TexturePngResource.hpp"
#include "raylib.h"


TexturePng::~TexturePng()
{
	Unload();
}

bool TexturePng::Load(const std::vector<uint8_t>& data)
{
	Image img = LoadImageFromMemory(".png", data.data(), (int)data.size());

	if (img.data == nullptr || img.width <= 0 || img.height <= 0)
	{
		std::cerr << "TexturePng: Failed to load the image." << std::endl;
		return false;
	}
	ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

	//size_t imgSize = img.width * img.height * 4;
	const int imgSize = GetPixelDataSize(img.width, img.height, img.format);
	if (imgSize <= 0)
	{
		std::cerr << "TexturePng::Load failed: invalid pixel data size\n";
		UnloadImage(img);
		return false;
	}

	m_imageData = (unsigned char*)malloc(imgSize);
	if (imgSize <= 0)
	{
		UnloadImage(img);
		return false;
	}
	
	memcpy(m_imageData, img.data, imgSize);
	m_width = img.width;
	m_height = img.height;
	m_channels = 4;

	UnloadImage(img);

	return true;
}

bool TexturePng::Unload()
{
	//unload un texture
	if (m_imageData)
	{
		//free(m_imageData);
		m_imageData = nullptr;
		m_height = m_width = m_channels = m_size = 0;
		return true;
	}

	return false;
}

const unsigned char* TexturePng::GetTexture()
{
	return m_imageData;
}
