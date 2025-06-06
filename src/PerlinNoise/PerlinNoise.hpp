#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <random>

class PerlinNoise
{
private:
    static const std::vector<glm::vec3> gradients;

    unsigned int m_resolution;
    const float m_width, m_height;
    std::vector<glm::vec3> m_node_gradients;
    glm::vec3 randomVector();
public:
    static std::mt19937 rng;
    static std::uniform_real_distribution<float> angle_dist;
    static std::uniform_real_distribution<float> z_dist;

    PerlinNoise(const float width, const float height, const unsigned int resolution, const bool select_gradients);

    float lerp(float a, float b, float t) const;
    float smoothStep(float t) const;
    void selectGradients();
    void randomGradients();
    void nextZGradients();
    float noise(const glm::vec2& xy, float z) const;
};
