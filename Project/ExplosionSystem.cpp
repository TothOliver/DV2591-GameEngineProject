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
    
}