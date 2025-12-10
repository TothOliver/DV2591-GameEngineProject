#include "Projectile.hpp"

void Projectile::Init(float x, float y, float dirX, float dirY, float speed, float lifetime)
{
    m_posX = x;
    m_posY = y;
    m_dirX = dirX;
    m_dirY = dirY;
    m_speed = speed;
    m_lifetime = lifetime;
    m_alive = true;
}

void Projectile::Update(float dt)
{
    if (!m_alive) return;

    m_posX += m_dirX * m_speed * dt;
    m_posY += m_dirY * m_speed * dt;

    m_lifetime -= dt;
    if (m_lifetime <= 0.0f)
        m_alive = false;
}

