#pragma once
#include "raylib.h"
#include "parser/tiny_obj_loader.h"

/*
* Converting to model so that there will be less crap in main
*/
static Model ConvertAttribToModel(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes)
{
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texcoords;

	for (const auto& shape : shapes)
	{
		for (const auto& idx : shape.mesh.indices)
		{
            // Vertices (x, y, z)
            int vIndex = 3 * idx.vertex_index;
            vertices.push_back(attrib.vertices[vIndex + 0]);
            vertices.push_back(attrib.vertices[vIndex + 1]);
            vertices.push_back(attrib.vertices[vIndex + 2]);

            // Normals (nx, ny, nz) if provided
            if (!attrib.normals.empty() && idx.normal_index >= 0)
            {
                int nIndex = 3 * idx.normal_index;
                normals.push_back(attrib.normals[nIndex + 0]);
                normals.push_back(attrib.normals[nIndex + 1]);
                normals.push_back(attrib.normals[nIndex + 2]);
            }
            else
            {
                // dummy normal, raylib can auto-generate later
                normals.push_back(0);
                normals.push_back(0);
                normals.push_back(0);
            }

            // Texture coordinates (u, v)
            if (!attrib.texcoords.empty() && idx.texcoord_index >= 0)
            {
                int tIndex = 2 * idx.texcoord_index;
                texcoords.push_back(attrib.texcoords[tIndex + 0]);
                texcoords.push_back(1.0f - attrib.texcoords[tIndex + 1]); // flip V
            }
            else
            {
                texcoords.push_back(0);
                texcoords.push_back(0);
            }
		}
	}

    Mesh mesh = { 0 };
    mesh.vertexCount = vertices.size() / 3;     // number of vertices
    mesh.triangleCount = mesh.vertexCount / 3;  // number of triangles

    // Allocate GPU data
    mesh.vertices = (float*)MemAlloc(vertices.size() * sizeof(float));
    mesh.normals = (float*)MemAlloc(normals.size() * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(texcoords.size() * sizeof(float));

    memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    memcpy(mesh.normals, normals.data(), normals.size() * sizeof(float));
    memcpy(mesh.texcoords, texcoords.data(), texcoords.size() * sizeof(float));

    // Upload data to GPU
    UploadMesh(&mesh, false);

    // Create model
    Model model = LoadModelFromMesh(mesh);

    return model;
}
