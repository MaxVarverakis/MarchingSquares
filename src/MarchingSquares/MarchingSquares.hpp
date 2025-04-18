#pragma once

#include <vector>
#include "../Point/Point.hpp"
#include "../Grid/Grid.hpp"

struct State
{
    bool v0, v1, v2, v3;

    bool hasEdge() { return !((v0 && v1 && v2 && v3) || !(v0 || v1 || v2 || v3)); }

    unsigned int state()
    {
        return 8 * v0 + 4 * v1 + 2 * v2 + v3;
    }
};

struct Cell
{
    unsigned int nw, ne, se, sw;
};

struct StateCell
{
    State state;
    Cell cell;
};

class MarchingSquares
{
private:
    float m_isolevel;
    bool m_interp;
    std::vector<Point> m_points;

    const std::vector<Point>& m_grid_points;
    const std::vector<float>& m_grid_values;
    State m_state;

    float lerp(const float a, const float b, const float t);
    void pushX(unsigned int active_node_idx, unsigned int inactive_node_idx);
    void pushY(unsigned int active_node_idx, unsigned int inactive_node_idx);

    bool active(const float value);

    void top(const StateCell& stateCell);
    void bottom(const StateCell& stateCell);
    void right(const StateCell& stateCell);
    void left(const StateCell& stateCell);
    void addEdgeVertices(State& state, const Cell& cell);

public:
    MarchingSquares(const float isolevel, const bool interp, const Grid& grid);

    void march(const Grid& grid);
    std::vector<Point>& points() { return m_points; }
    std::vector<float> positions();
    float getIsolevel() { return m_isolevel; }
    void setIsolevel(const float isolevel) { m_isolevel = isolevel; }
    void clear();
};
