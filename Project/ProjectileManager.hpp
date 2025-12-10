#pragma once
#include "Projectile.hpp"
#include "MemoryManager/Memory.hpp"
#include <vector>
#include <string>

class ProjectileManager
{
public:
    void Initialize(size_t maxProjectiles);

    Projectile* Create(float x, float y, float z,
        float dx, float dy, float dz,
        float speed, float lifetime,
        const std::string& meshGUID, const std::string& textureGUID);

    void Update(float dt);
    void Shutdown();

    const std::vector<Projectile*>& GetProjectiles() const { return m_projectiles; }

private:
    std::vector<Projectile*> m_projectiles;
    std::vector<Projectile*> m_freeProjectiles;
};