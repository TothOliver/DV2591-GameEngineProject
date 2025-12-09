#include "AssetManager/MeshObjResource.hpp"
#include <vector>
#include <sstream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "AssetManager/parser/tiny_obj_loader.h"

MeshObj::~MeshObj()
{
}

bool MeshObj::Load(const std::vector<uint8_t>& data)
{
	//Load mesh :)
	m_size = data.size();
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;
	
	std::string objText = (char*)data.data();
	std::istringstream objStream(objText);

	bool ok = tinyobj::LoadObj(&m_attrib, &m_shapes, &materials, &warn, &err, &objStream);

	if (!ok)
	{
		std::cerr << "MeshObj, TinyObj error: " << err << std::endl;
		return false;
	}

	if (!warn.empty())
	{
		std::cout << "MeshObj, TinyObj warning: " << warn << std::endl;
	}

	return true;
}

bool MeshObj::Unload()
{
	//Unload mesh :O
	tinyobj::attrib_t empty;
	m_attrib = empty;
	m_size = 0;
	return true;
}

tinyobj::attrib_t MeshObj::GetAttrib() const
{
	return m_attrib;
}

std::vector<tinyobj::shape_t> MeshObj::GetShapes() const
{
	return m_shapes;
}
