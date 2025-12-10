#pragma once
#include <cmath>

class Projectile
{
public:
    Projectile() = default;

    void Init(float x, float y, float dirX, float dirY, float speed, float lifetime);

    void Update(float dt);

    bool IsAlive() const { return m_alive; }

private:
    float m_posX = 0;
    float m_posY = 0;
    float m_dirX = 0;
    float m_dirY = 0;
    float m_speed = 0;
    float m_lifetime = 0;
    bool m_alive = false;
};
