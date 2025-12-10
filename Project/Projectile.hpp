#pragma once
#include <cmath>
#include <string>

class Projectile
{
public:
    Projectile() = default;

    void Init(float x, float y, float z, float dirX, float dirY, float dirZ,
        float speed, float lifetime,
        const std::string& meshGUID, const std::string& textureGUID);

    void Update(float dt);

    bool IsAlive() const { return m_alive; }

    // Getters for rendering
    float GetPosX() const { return m_posX; }
    float GetPosY() const { return m_posY; }
    float GetPosZ() const { return m_posZ; }
    float GetDirX() const { return m_dirX; }
    float GetDirY() const { return m_dirY; }
    float GetDirZ() const { return m_dirZ; }

    const std::string& GetMeshGUID() const { return m_meshGUID; }
    const std::string& GetTextureGUID() const { return m_textureGUID; }

    // Optional: bounds checking for 3D
    bool IsOutOfBounds(float minX, float minY, float minZ,
        float maxX, float maxY, float maxZ) const;

private:
    float m_posX = 0.0f;
    float m_posY = 0.0f;
    float m_posZ = 0.0f;
    float m_dirX = 0.0f;
    float m_dirY = 0.0f;
    float m_dirZ = 0.0f;
    float m_speed = 0.0f;
    float m_lifetime = 0.0f;
    bool m_alive = false;
    std::string m_meshGUID;
    std::string m_textureGUID;
};