#pragma once

#include <glm/glm.hpp>
#include <vector>

class Particle
{
private:
    float m_radius;
    glm::vec2 m_position;
    glm::vec2 m_velocity;
public:
    Particle(const float radius, const glm::vec2 position, const glm::vec2 velocity)
        : m_radius { radius }
        , m_position { position }
        , m_velocity { velocity }
    {}

    float radius() { return m_radius; }
    glm::vec2 position() { return m_position; }
    glm::vec2 velocity() { return m_velocity; }

    void setPosition(const glm::vec2 position) { m_position = position; }
    void setVelocity(const glm::vec2 velocity) { m_velocity = velocity; }
    
    void updatePosition(const float dt);
    void applyBoundaryCondition(const float width, const float height);
    void evolve(const float width, const float height, const float dt);
};

// struct Particles
// {
// // private:
// //     std::vector<glm::vec2> m_positions;
// //     std::vector<glm::vec2> m_velocities;
// //     std::vector<float> m_radii;
// // public:

//     std::vector<const Particle&> m_particles;

//     Particles(std::vector<Particle> particles)
//     {
//         for (const Particle& particle : particles)
//         {
//             addParticle(particle);
//         }
//     }

//     void addParticle(const Particle& particle) { m_particles.push_back(particle); }
// };
