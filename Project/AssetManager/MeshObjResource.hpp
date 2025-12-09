#pragma once
#include <iostream>
#include "IResource.hpp"
#include "parser/tiny_obj_loader.h"

class MeshObj : public IResource
{
public:
	MeshObj(std::string GUID) : IResource(GUID, ResourceType::Mesh) {}
	~MeshObj();

	bool Load(const std::vector<uint8_t>& data) override;
	bool Unload() override;

	tinyobj::attrib_t GetAttrib() const;
	std::vector<tinyobj::shape_t> GetShapes() const;

private:
	tinyobj::attrib_t m_attrib;
	std::vector<tinyobj::shape_t> m_shapes;
};