#include "MarchingSquares.hpp"
#include <iostream>

MarchingSquares::MarchingSquares(const float isolevel, const bool interp, const Grid& grid)
    : m_isolevel { isolevel }
    , m_interp { interp }
    , m_grid_points { grid.points() }
    , m_grid_values {  grid.values() }
{
    march(grid);
}

void MarchingSquares::march(const Grid& grid)
{
    clear();
    
    // row major order
    m_state = { active(m_grid_values[0]), 0, 0, active(m_grid_values[grid.resolution()]) };

    for (unsigned int y_i = 0; y_i < grid.resolution()-1; ++y_i)
    {
        for (unsigned int x_i = 0; x_i < grid.resolution()-1; ++x_i)
        {
            unsigned int row_offset { grid.resolution() * y_i };
            unsigned int nw_idx { row_offset + x_i };
            unsigned int sw_idx { nw_idx + grid.resolution() };
            m_state.v1 = active(m_grid_values[nw_idx + 1]); // top right
            m_state.v2 = active(m_grid_values[sw_idx + 1]); // bottom right

            // std::cout << m_state.v0 << m_state.v1 << m_state.v2 << m_state.v3 << '\n';

            if (m_state.hasEdge())
            {
                addEdgeVertices(
                    m_state,
                    {
                        nw_idx,     // nw
                        nw_idx + 1, // ne
                        sw_idx + 1, // se
                        sw_idx      // sw
                    }
                    );
            }

            // set left indices for next iteration
            if (x_i == grid.resolution()-2)
            {
                m_state.v0 = active(m_grid_values[ grid.resolution() * ( y_i + 1 ) ]);
                m_state.v3 = active(m_grid_values[ grid.resolution() * ( y_i + 2 ) ]);
            }
            else
            {
                m_state.v0 = m_state.v1;
                m_state.v3 = m_state.v2;
            }
        }
    }
}

std::vector<float> MarchingSquares::positions()
{
    std::vector<float> positions;
    positions.reserve(2 * m_points.size());

    for (const Point& point : m_points)
    {
        const glm::vec2& pos { point.position() };

        positions.push_back(pos.x);
        positions.push_back(pos.y);
    }

    return positions;
}

float MarchingSquares::lerp(const float a, const float b, const float t)
{
    return a + t * (b - a);
}

bool MarchingSquares::active(const float value)
{
    return (value < m_isolevel) ? 0 : 1;
}

void MarchingSquares::pushX(unsigned int active_node_idx, unsigned int inactive_node_idx)
{
    if (m_interp)
    {
        m_points.emplace_back(
            lerp(
                    m_grid_points[active_node_idx].position().x,
                    m_grid_points[inactive_node_idx].position().x,
                    1 - (m_isolevel - m_grid_values[inactive_node_idx]) / (m_grid_values[active_node_idx] - m_grid_values[inactive_node_idx])
                ),
                m_grid_points[active_node_idx].position().y
            );
    }
    else
    {
        m_points.emplace_back( (m_grid_points[active_node_idx].position().x + m_grid_points[inactive_node_idx].position().x) / 2 , m_grid_points[active_node_idx].position().y);
    }
}

void MarchingSquares::pushY(unsigned int active_node_idx, unsigned int inactive_node_idx)
{
    if (m_interp)
    {
        m_points.emplace_back(
            m_grid_points[active_node_idx].position().x,
            lerp(
                    m_grid_points[active_node_idx].position().y,
                    m_grid_points[inactive_node_idx].position().y,
                    1 - (m_isolevel - m_grid_values[inactive_node_idx]) / (m_grid_values[active_node_idx] - m_grid_values[inactive_node_idx])
                )
            );
    }
    else
    {
        m_points.emplace_back(m_grid_points[active_node_idx].position().x, (m_grid_points[active_node_idx].position().y + m_grid_points[inactive_node_idx].position().y) / 2);
    }
}

void MarchingSquares::top(const StateCell& stateCell)
{
    unsigned int nw_idx { stateCell.cell.nw };
    unsigned int ne_idx { stateCell.cell.ne };

    unsigned int active_node_idx { stateCell.state.v0 ? nw_idx : ne_idx };
    unsigned int inactive_node_idx { stateCell.state.v0 ? ne_idx : nw_idx };

    pushX(active_node_idx, inactive_node_idx);
}

void MarchingSquares::bottom(const StateCell& stateCell)
{
    unsigned int se_idx { stateCell.cell.se };
    unsigned int sw_idx { stateCell.cell.sw };

    unsigned int active_node_idx { stateCell.state.v2 ? se_idx : sw_idx };
    unsigned int inactive_node_idx { stateCell.state.v2 ? sw_idx : se_idx };

    pushX(active_node_idx, inactive_node_idx);
}

void MarchingSquares::right(const StateCell& stateCell)
{
    unsigned int ne_idx { stateCell.cell.ne };
    unsigned int se_idx { stateCell.cell.se };

    unsigned int active_node_idx { stateCell.state.v1 ? ne_idx : se_idx };
    unsigned int inactive_node_idx { stateCell.state.v1 ? se_idx : ne_idx };

    pushY(active_node_idx, inactive_node_idx);
}

void MarchingSquares::left(const StateCell& stateCell)
{
    unsigned int nw_idx { stateCell.cell.nw };
    unsigned int sw_idx { stateCell.cell.sw };

    unsigned int active_node_idx { stateCell.state.v0 ? nw_idx : sw_idx };
    unsigned int inactive_node_idx { stateCell.state.v0 ? sw_idx : nw_idx };

    pushY(active_node_idx, inactive_node_idx);
}

void MarchingSquares::addEdgeVertices(State& state, const Cell& cell)
{
    StateCell sc { state, cell };
    switch (state.state())
    {
        case 1:
            left(sc);
            bottom(sc);
            return;
        
        case 2:
            right(sc);
            bottom(sc);
            return;

        case 3:
            left(sc);
            right(sc);
            return;

        case 4:
            top(sc);
            right(sc);
            return;

        case 5:
            left(sc);
            top(sc);

            bottom(sc);
            right(sc);
            return;

        case 6:
            top(sc);
            bottom(sc);
            return;

        case 7:
            left(sc);
            top(sc);
            return;

        case 8:
            top(sc);
            left(sc);
            return;

        case 9:
            top(sc);
            bottom(sc);
            return;

        case 10:
            left(sc);
            bottom(sc);

            top(sc);
            right(sc);
            return;

        case 11:
            top(sc);
            right(sc);
            return;

        case 12:
            left(sc);
            right(sc);
            return;

        case 13:
            right(sc);
            bottom(sc);
            return;

        case 14:
            left(sc);
            bottom(sc);
            return;
    }
}

void MarchingSquares::clear()
{
    m_points.clear();
}
