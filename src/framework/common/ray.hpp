#pragma once
#include <SFML/System/Vector2.hpp>
#include "engine/common/math.hpp"
#include "engine/common/utils.hpp"


struct Ray
{
    float   d[2];
    float   start_pos[2];
    int32_t sgn[2];
    int32_t i_pos[2];
    float   inv_d[2];
    float   t_d[2];
    float   t_max[2];
    float   dist;

    Ray() = default;

    Ray(sf::Vector2f start, sf::Vector2f direction, sf::Vector2i cell)
        : d{ direction.x, direction.y }
        , start_pos{ start.x, start.y }
        , sgn{ to<int32_t>(Math::sign(direction.x)), to<int32_t>(Math::sign(direction.y)) }
        , i_pos{ cell.x, cell.y }
        , inv_d{ 1.0f / direction.x, 1.0f / direction.y }
        , t_d{ std::abs(inv_d[0]), std::abs(inv_d[1]) }
        , dist(0.0f)
    {
        t_max[0] = (to<float>(i_pos[0] + (sgn[0] > 0)) - start_pos[0]) * inv_d[0];
        t_max[1] = (to<float>(i_pos[1] + (sgn[1] > 0)) - start_pos[1]) * inv_d[1];
    }

    sf::Vector2i computeNextCell()
    {
        const uint8_t min_coord = t_max[1] < t_max[0];
        // Advance in grid
        t_max[min_coord] += t_d[min_coord];
        i_pos[min_coord] += sgn[min_coord];
        dist += t_d[min_coord];
        return { i_pos[0], i_pos[1] };
    }
};