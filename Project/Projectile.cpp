#include "Projectile.hpp"

void Projectile::Init(float x, float y, float z, float dirX, float dirY, float dirZ,
    float speed, float lifetime,
    const std::string& meshGUID, const std::string& textureGUID)
{
    m_posX = x;
    m_posY = y;
    m_posZ = z;

    // Normalize
    float length = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
    if (length > 0.0f)
    {
        m_dirX = dirX / length;
        m_dirY = dirY / length;
        m_dirZ = dirZ / length;
    }
    else
    {
        m_dirX = 0.0f;
        m_dirY = 0.0f;
        m_dirZ = 1.0f;  // Default forward
    }

    m_speed = speed;
    m_lifetime = lifetime;
    m_alive = true;
    m_meshGUID = meshGUID;
    m_textureGUID = textureGUID;
}

void Projectile::Update(float dt)
{
    if (!m_alive) return;

    m_posX += m_dirX * m_speed * dt;
    m_posY += m_dirY * m_speed * dt;
    m_posZ += m_dirZ * m_speed * dt;

    m_lifetime -= dt;
    if (m_lifetime <= 0.0f)
        m_alive = false;
}
