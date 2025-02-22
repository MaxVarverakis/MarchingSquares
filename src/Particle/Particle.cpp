#include "Particle.hpp"

void Particle::updatePosition(const float dt)
{
    m_position += m_velocity * dt;
}

void Particle::applyBoundaryCondition(const float width, const float height)
{
    // assumes domain is [0, width] x [0, height]
    if ( (m_position.x - m_radius < 0.0f) || (m_position.x + m_radius > width) )
    {
        m_velocity.x *= -1.0f;
    }
    else if ( (m_position.y - m_radius < 0.0f) || (m_position.y + m_radius > height) )
    {
        m_velocity.y *= -1.0f;
    }
}

void Particle::evolve(const float width, const float height, const float dt)
{
    applyBoundaryCondition(width, height);
    updatePosition(dt);
}
