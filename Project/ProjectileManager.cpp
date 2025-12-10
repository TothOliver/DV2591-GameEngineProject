#include "ProjectileManager.hpp"

void ProjectileManager::Initialize(size_t maxProjectiles)
{
    InitPool(sizeof(Projectile), maxProjectiles, alignof(Projectile));
    m_projectiles.reserve(maxProjectiles);
    m_freeProjectiles.reserve(maxProjectiles);
}

Projectile* ProjectileManager::Create(float x, float y, float z,
    float dx, float dy, float dz,
    float speed, float lifetime,
    const std::string& meshGUID,
    const std::string& textureGUID)
{
    Projectile* proj = nullptr;

    if (!m_freeProjectiles.empty())
    {
        proj = m_freeProjectiles.back();
        m_freeProjectiles.pop_back();
        proj->Init(x, y, z, dx, dy, dz, speed, lifetime, meshGUID, textureGUID);
    }
    else
    {
        void* mem = PoolAlloc();
        if (!mem) return nullptr;

        proj = new(mem) Projectile();
        proj->Init(x, y, z, dx, dy, dz, speed, lifetime, meshGUID, textureGUID);
    }

    m_projectiles.push_back(proj);
    return proj;
}

void ProjectileManager::Update(float dt)
{
    for (size_t i = 0; i < m_projectiles.size(); )
    {
        Projectile* p = m_projectiles[i];
        p->Update(dt);

        if (!p->IsAlive())
        {
            m_freeProjectiles.push_back(p);
            m_projectiles[i] = m_projectiles.back();
            m_projectiles.pop_back();
        }
        else
        {
            ++i;
        }
    }
}

void ProjectileManager::Shutdown()
{
    for (Projectile* p : m_projectiles)
        p->~Projectile();

    for (Projectile* p : m_freeProjectiles)
        p->~Projectile();

    ShutdownMemory();

    m_projectiles.clear();
    m_freeProjectiles.clear();
}