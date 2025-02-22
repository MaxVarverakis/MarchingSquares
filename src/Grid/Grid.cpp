#include "Grid.hpp"

Grid::Grid(const float width, const float height, const unsigned int resolution, float (*f)(const glm::vec2&))
    : m_resolution { resolution }
{
    m_points.reserve(resolution * resolution);
    m_values.reserve(resolution * resolution);

    createPoints(width, height);
    for (const Point& point : m_points)
    {
        m_values.emplace_back(f(point.position()));
    }
}

Grid::Grid(const float width, const float height, const unsigned int resolution, float (*f)(const glm::vec2&, const float t))
    : m_resolution { resolution }
{
    m_points.reserve(resolution * resolution);
    m_values.reserve(resolution * resolution);

    createPoints(width, height);
    for (const Point& point : m_points)
    {
        m_values.emplace_back(f(point.position(), 0.0f));
    }
}

Grid::Grid(const float width, const float height, const unsigned int resolution, std::vector<Particle>& particles)
    : m_resolution { resolution }
    , m_values(resolution * resolution, 0.0f)
{
    m_points.reserve(resolution * resolution);

    createPoints(width, height);
    assignValues(particles);
}

void Grid::assignValues(float (*f)(const glm::vec2&))
{
    for (unsigned int i = 0; i < m_values.size(); ++i)
    {
        m_values[i] = f(m_points[i].position());
    }
}

void Grid::assignValues(float (*f)(const glm::vec2&, const float t), const float t)
{
    for (unsigned int i = 0; i < m_values.size(); ++i)
    {
        m_values[i] = f(m_points[i].position(), t);
    }
}

void Grid::assignValues(std::vector<Particle>& particles)
{
    for (unsigned int i = 0; i < m_points.size(); ++i)
    {
        const glm::vec2& location = m_points[i].position();
        float& value = m_values[i];
        value = 0.0f;
        for (Particle& particle : particles)
        {
            // possible parallelization: use different thread for each particle
            value += particle.radius() / glm::length(location - particle.position());
        }
    }
}

void Grid::createPoints(const float width, const float height)
{
    // Determine the aspect ratio
    const float aspectRatio = width / height;

    float dx { width / (m_resolution - 1) };
    float dy { height / (static_cast<unsigned int>(m_resolution / aspectRatio) - 1) };

    for (unsigned int y_i = 0; y_i < m_resolution; ++y_i)
    {
        for (unsigned int x_i = 0; x_i < m_resolution; ++x_i)
        {
            m_points.emplace_back(Point(static_cast<float>(x_i) * dx, static_cast<float>(y_i) * dy));
        }
    }
}
