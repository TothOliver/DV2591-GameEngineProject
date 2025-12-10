#pragma once
#include "raylib.h"
#include <vector>
#include <cstddef>

class StackAllocator;

struct Explosion
{
    Vector3 position;
    float age;
    float duration;
    float radius;
};

struct ExplosionVertex
{
    Vector3 position;
    float size;
    Color color;
};

class ExplosionSystem
{
public:
    ExplosionSystem(StackAllocator& frameAllocator);
    ~ExplosionSystem();

    void AddExplosion(const Vector3& position, float radius, float duration);
    void Update(float dt);
    void BuildRendererData();

    const ExplosionVertex* GetVertices() const {return m_vertices; }
    size_t GetVertexCount() const {return m_vertexCounter; }

private:
    StackAllocator& m_frameAllocator;
    std::vector<Explosion> m_explosions;

    ExplosionVertex* m_vertices = nullptr;
    size_t m_vertexCounter = 0;

    static constexpr int PARTICLES_PER_EXPLOSION = 32;
};