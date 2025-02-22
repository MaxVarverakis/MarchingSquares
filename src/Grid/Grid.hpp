#pragma once

#include <vector>
#include "../Point/Point.hpp"
#include "../Particle/Particle.hpp"

class Grid
{
private:
    // initialize grid of points
    unsigned int m_resolution;
    std::vector<Point> m_points;
    std::vector<float> m_values;

    void createPoints(const float width, const float height);
public:
    Grid(const float width, const float height, const unsigned int resolution, float (*f)(const glm::vec2&));
    Grid(const float width, const float height, const unsigned int resolution, float (*f)(const glm::vec2&, const float t));
    Grid(const float width, const float height, const unsigned int resolution, std::vector<Particle>& particles);

    void assignValues(float (*f)(const glm::vec2&));
    void assignValues(float (*f)(const glm::vec2&, const float t), const float t);
    void assignValues(std::vector<Particle>& particles);

    unsigned int size() { return m_resolution * m_resolution; }
    unsigned int resolution() const { return m_resolution; }
    const std::vector<Point>& points() const { return m_points; }
    const std::vector<float>& values() const { return m_values; }
    void setValue(float val, unsigned int idx) { m_values[idx] = val; }
};
