#pragma once

#include <glm/glm.hpp>

class Point
{
private:
    glm::vec2 m_position;

public:
    Point(const float v1, const float v2);

    const glm::vec2& position() const { return m_position; }

    void setPosition(const glm::vec2& pos) { m_position = pos; }
    void setPosition(const float v1, const float v2) { m_position = glm::vec2(v1, v2); }
};
