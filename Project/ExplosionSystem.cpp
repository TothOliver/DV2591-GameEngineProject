#include "ExplosionSystem.hpp"
#include "MemoryManager/StackAllocator.hpp"
#include <algorithm>
#include <cmath>

ExplosionSystem::ExplosionSystem(StackAllocator& frameAllocator)
    : m_frameAllocator(frameAllocator)
{
    m_explosions.reserve(50);
}

void ExplosionSystem::AddExplosion(const Vector3& position, float radius, float duration)
{
    Explosion ex;
    ex.position = position;
    ex.age = 0.0f;
    ex.duration = duration;
    ex.radius = radius;

    m_explosions.push_back(ex);
}

void ExplosionSystem::Update(float dt)
{
    for(Explosion& ex : m_explosions){
        ex.age += dt;
    }

    m_explosions.erase(
        std::remove_if(m_explosions.begin(), m_explosions.end(), 
        [](const Explosion& ex){
            return ex.age >= ex.duration;
        }),
        m_explosions.end()
    );
}

void ExplosionSystem::BuildRendererData()
{
    if(m_explosions.empty() || m_explosions.size() == 0){
        m_vertices = nullptr;
        m_vertexCounter = 0;
        return;
    }

    const size_t explosionCount = m_explosions.size();
    const size_t totalParticales = explosionCount * PARTICLES_PER_EXPLOSION;
    const size_t bytesNeeded = totalParticales * sizeof(ExplosionVertex);
    void* mem = m_frameAllocator.Allocate(bytesNeeded, alignof(ExplosionVertex));

    if(!mem){
        m_vertices = nullptr;
        m_vertexCounter = 0;
         std::cerr << "Error, ExplosionSystem Stack allocation failed;" << std::endl;
    }

    m_vertices = static_cast<ExplosionVertex*>(mem);
    m_vertexCounter = totalParticales;

    size_t writeIndex = 0;

    for(const Explosion& ex : m_explosions)
    {
        const float t = (ex.duration > 0.0f) ? (ex.age / ex.duration) : 1.0f;
        const float clampedT = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
        const float currentRadius = ex.radius * clampedT;

        for(int i = 0; i < PARTICLES_PER_EXPLOSION; i++)
        {
            const float angle = (2.0f * PI * i) / PARTICLES_PER_EXPLOSION;

            Vector3 offset;
            offset.x = cosf(angle) * currentRadius;
            offset.z = sinf(angle) * currentRadius;
            offset.y = 0.3f * clampedT;

            ExplosionVertex& v = m_vertices[writeIndex++];

            v.position = Vector3{ ex.position.x + offset.x,
                                  ex.position.y + offset.y,
                                  ex.position.z + offset.z 
                                };
            v.size = 0.4f * (1.0f - clampedT) + 0.1f;

            unsigned char alpha = static_cast<unsigned char>(255.0f * (1.0f - clampedT));
            v.color = Color{
                        255,
                        static_cast<unsigned char>(150 + 105 * (1.0f - clampedT)),
                        0,
                        alpha
                    };
        }
    }
}