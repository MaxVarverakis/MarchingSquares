#include "PerlinNoise.hpp"

#include <cstdlib>
#include <iostream>

const std::vector<glm::vec3> PerlinNoise::gradients = {
    {1.0f,1.0f,0.0f}, {-1.0f,1.0f,0.0f}, {1.0f,-1.0f,0.0f}, {-1.0f,-1.0f,0.0f},
    {1.0f,0.0f,1.0f}, {-1.0f,0.0f,1.0f}, {1.0f,0.0f,-1.0f}, {-1.0f,0.0f,-1.0f},
    {0.0f,1.0f,1.0f}, {0.0f,-1.0f,1.0f}, {0.0f,1.0f,-1.0f}, {0.0f,-1.0f,-1.0f}
};

std::mt19937 PerlinNoise::rng(std::random_device{}());
std::uniform_real_distribution<float> PerlinNoise::angle_dist(0.0f, 2.0f * static_cast<float>(M_PI));
std::uniform_real_distribution<float> PerlinNoise::z_dist(-1.0f, 1.0f);

PerlinNoise::PerlinNoise(const float width, const float height, const unsigned int resolution, const bool select_gradients)
    : m_resolution { resolution }
    , m_width { width }
    , m_height { height }
{
    m_node_gradients.reserve(resolution * resolution);
    select_gradients ? selectGradients() : randomGradients();
}

glm::vec3 PerlinNoise::randomVector()
{
    float z { PerlinNoise::z_dist(PerlinNoise::rng) };
    float theta { PerlinNoise::angle_dist(PerlinNoise::rng)};
    float r { sqrtf(1.0f - z*z) };

    return glm::vec3( r * cos(theta), r * sin(theta), z );
}

float PerlinNoise::lerp(float a, float b, float t) const
{
    return a + (b - a) * t;
}

float PerlinNoise::smoothStep(float t) const
{
    return ((6*t - 15)*t + 10)*t*t*t;
}

void PerlinNoise::selectGradients()
{
    // seed rng
    std::srand(static_cast<unsigned int>(std::time({}))); // use current time as seed

    for (unsigned int i = 0; i < m_resolution * m_resolution; ++i)
    {
        unsigned int idx = static_cast<unsigned>(rand() % 12);
        m_node_gradients.push_back(gradients[idx]);
    }
}

void PerlinNoise::randomGradients()
{
    for (unsigned int i = 0; i < 2 * m_resolution * m_resolution; ++i)
    {
        m_node_gradients.emplace_back(randomVector());
    }
}

void PerlinNoise::nextZGradients()
{
    for (unsigned int i = 0; i < m_resolution * m_resolution; ++i)
    {
        m_node_gradients[i] = m_node_gradients[i + m_resolution * m_resolution];
        m_node_gradients[i + m_resolution * m_resolution] = randomVector();
    }
}

float PerlinNoise::noise(const glm::vec2& xy, float z) const
{
    // Determine the aspect ratio
    const float aspectRatio = m_width / m_height;

    float dx { m_width / (m_resolution - 1) };
    float dy { m_height / (static_cast<unsigned int>(m_resolution / aspectRatio) - 1) };
    // dz = 1

    unsigned int x_idx { static_cast<unsigned int>(floor(xy.x / dx)) };
    unsigned int y_idx { static_cast<unsigned int>(floor(xy.y / dy)) };
    float xf { xy.x / dx - x_idx };
    float yf { xy.y / dy - y_idx };
    float zf = { z - floor(z) };

    // displacement vectors
    // --------------------
    // assumes local bottom left coordinate is origin
    const glm::vec3 bottomLeft { xf, yf, zf };
    const glm::vec3 topLeft { xf, yf - 1.0f, zf };
    const glm::vec3 bottomRight { xf - 1.0f, yf, zf };
    const glm::vec3 topRight { xf - 1.0f, yf - 1.0, zf };
    
    const glm::vec3 bottomLeftCeiling { xf, yf, zf - 1.0f };
    const glm::vec3 topLeftCeiling { xf, yf - 1.0f, zf - 1.0f };
    const glm::vec3 bottomRightCeiling { xf - 1.0f, yf, zf - 1.0f };
    const glm::vec3 topRightCeiling { xf - 1.0f, yf - 1.0, zf - 1.0f };

    return (
            lerp(
                lerp(
                    lerp(
                        glm::dot(bottomLeft, m_node_gradients[x_idx + m_resolution * (y_idx + 0)]),
                        glm::dot(bottomRight, m_node_gradients[x_idx + 1 + m_resolution * (y_idx + 0)]), 
                        smoothStep(xf)
                    ),
                    lerp(
                        glm::dot(topLeft, m_node_gradients[x_idx + m_resolution * (y_idx + 1)]),
                        glm::dot(topRight, m_node_gradients[x_idx + 1 + m_resolution * (y_idx + 1)]), 
                        smoothStep(xf)
                    ),
                    smoothStep(yf)
                ),
                lerp(
                    lerp(
                        glm::dot(bottomLeftCeiling, m_node_gradients[x_idx + m_resolution * (y_idx + 0) + m_resolution * m_resolution]),
                        glm::dot(bottomRightCeiling, m_node_gradients[x_idx + 1 + m_resolution * (y_idx + 0) + m_resolution * m_resolution]), 
                        smoothStep(xf)
                    ),
                    lerp(
                        glm::dot(topLeftCeiling, m_node_gradients[x_idx + m_resolution * (y_idx + 1) + m_resolution * m_resolution]),
                        glm::dot(topRightCeiling, m_node_gradients[x_idx + 1 + m_resolution * (y_idx + 1) + m_resolution * m_resolution]), 
                        smoothStep(xf)
                    ),
                    smoothStep(yf)
                ),
                zf
            )
        + 1) / 2;

}
