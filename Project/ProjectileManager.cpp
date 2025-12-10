#include "ProjectileManager.hpp"

void ProjectileManager::Initialize(size_t maxProjectiles)
{
    InitPool(sizeof(Projectile), maxProjectiles, alignof(Projectile));
    m_projectiles.reserve(maxProjectiles);
}

Projectile* ProjectileManager::Create(float x, float y, float dx, float dy, float speed, float lifetime)
{
    Projectile* proj = nullptr;

    if (!m_freeProjectiles.empty())
    {
        proj = m_freeProjectiles.back();
        m_freeProjectiles.pop_back();
        proj->Init(x, y, dx, dy, speed, lifetime);
    }
    else
    {
        void* mem = PoolAlloc();
        if (!mem) return nullptr;
        proj = new(mem) Projectile();  // placement new
        proj->Init(x, y, dx, dy, speed, lifetime);
    }

    m_projectiles.push_back(proj);
    return proj;
}


void ProjectileManager::Update(float dt)
{
    for (int i = 0; i < m_projectiles.size(); i++)
    {
        Projectile* p = m_projectiles[i];
        p->Update(dt);

        if (!p->IsAlive())
        {
            //return to free pool
            m_freeProjectiles.push_back(p);

            m_projectiles[i] = m_projectiles.back();
            m_projectiles.pop_back();
            i--;
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
