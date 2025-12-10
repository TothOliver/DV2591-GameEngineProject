#pragma once
#include "Projectile.hpp"
#include "MemoryManager/Memory.hpp"
#include <vector>

class ProjectileManager
{
public:
    void Initialize(size_t maxProjectiles);
    Projectile* Create(float x, float y, float dx, float dy, float speed, float lifetime);
    void Update(float dt);
    void Shutdown();

private:
    std::vector<Projectile*> m_projectiles;
    std::vector<Projectile*> m_freeProjectiles;
};
