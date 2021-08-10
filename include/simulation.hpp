#pragma once
#include <SFML/Graphics.hpp>


struct PositionHistory
{
    std::vector<sf::Vector2f> position_history;
    uint32_t current_position_idx = 0;
    uint32_t history_size;
    
    sf::VertexArray va;
    
    PositionHistory(uint32_t size, sf::Vector2f first_position = {})
        : position_history(size, first_position)
        , history_size(size)
        , va(sf::LineStrip, size)
    {
    }
    
    void add(sf::Vector2f pos)
    {
        position_history[current_position_idx] = pos;
        current_position_idx                   = (++current_position_idx) % history_size;
    }
    
    sf::VertexArray getVA()
    {
        for (uint32_t i(0); i < history_size; ++i) {
            const uint32_t real_idx = (i + current_position_idx) % history_size;
            const float ratio = i / float(history_size);
            va[i].position = position_history[real_idx];
            const sf::Vector3f color = ratio * stable_color;
            va[i].color = sf::Color(static_cast<sf::Uint8>(color.x), static_cast<sf::Uint8>(color.y), static_cast<sf::Uint8>(color.z));
        }

        return va;
    }
};


struct Object
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;

    bool stable;
    int32_t stableCount;

    Object()
    {
    }

    Object(float x, float y, float r)
        : position(x, y)
        , velocity(static_cast<float>(rand() % 8 - 4), static_cast<float>(rand() % 8 - 4))
        , radius(r)
    {
        stableCount = 0;
    }
    
    void update(float dt, float speed_down_factor)
    {
        position += (dt / speed_down_factor) * currentBall.velocity;
    }
};

bool update(std::vector<Ball>& balls, float speed)
{
    bool stable = true;

    const uint32_t nBalls = static_cast<uint32_t>(balls.size());
    const float dt = 0.008f;
    const float attraction_force = 50.0f;
    const float attraction_force_bug = 0.01f;
    const sf::Vector2f center_position(WIN_WIDTH * 0.5f, WIN_HEIGHT * 0.5f);
    const float attractor_threshold = 50.0f;

    for (uint32_t i(0); i<nBalls; i++) {
        Ball& current_ball = balls[i];
        // Attraction to center
        const sf::Vector2f to_center = center_position - current_ball.position;
        const float dist_to_center = length(to_center);
        current_ball.velocity += attraction_force_bug * to_center;

        for (uint32_t k=i+1; k<nBalls; k++) {
            Ball& collider = balls[k];
            const sf::Vector2f collide_vec = current_ball.position - collider.position;
            const float dist = sqrt(collide_vec.x*collide_vec.x + collide_vec.y*collide_vec.y);

            const float minDist = current_ball.r+collider.r;

            if (dist < minDist) {
                stable = false;

                current_ball.stable = false;
                collider.stable = false;

                const sf::Vector2f collide_axe = collide_vec / dist;

                current_ball.position += 0.5f * (minDist - dist) * collide_axe;
                collider.position -= 0.5f * (minDist - dist) * collide_axe;
            }
        }
    }

    for (uint32_t i(0); i<nBalls; i++)
    {
        if(balls[i].stable)
            balls[i].stableCount++;
        else
            balls[i].stableCount = 0;
    }

    return stable;
}

void updatePos(std::vector<Ball>& balls, float speedDownFactor, float& speedDownCounter)
{
    const float dt = 0.016f;
    for (Ball& currentBall : balls) {
       currentBall.position += (dt / speedDownFactor) * currentBall.velocity;
    }

    speedDownCounter--;
}


struct Solver
{
    std::vector<Object> objects;
    float speed_down_factor = 1.0f;
    uint32_t position_history_size = 100;
    
    const Object* getAt(sf::Vector2f position)
    {
        for (const Object& o : objects) {
            const sf::Vector2f v = position - o.position;
            const float dist = sqrt(v.x*v.x + v.y * v.y);
            if (dist < ball.r) {
                return &ball;
            }
        }

        return nullptr;
    }
    
    void update(float dt)
    {
        for (Object& o : objects) {
            o.update(dt, speed_down_factor);
        }
    }
};
