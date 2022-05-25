#pragma once
#include <SFML/Graphics.hpp>


struct ViewportHandler
{
    struct State
    {
        sf::Vector2f center;
        sf::Vector2f offset;
        float zoom;
        bool clicking;
        sf::Vector2f mouse_position;
        sf::Vector2f mouse_world_position;
        sf::Transform transform;

        State(sf::Vector2f render_size, const float base_zoom = 1.0f)
            : center(render_size.x * 0.5f, render_size.y * 0.5f)
            , offset(center / base_zoom)
            , zoom(base_zoom)
            , clicking(false)
        {}

        void updateState()
        {
            const float z = zoom;
            const float inv_z = 1.0f / z;
            transform = sf::Transform::Identity;
            transform.translate(center);
            transform.scale(z, z);
            transform.translate(-offset);
        }

        void updateMousePosition(sf::Vector2f new_position)
        {
            mouse_position = new_position;
            const sf::Vector2f pos(static_cast<float>(new_position.x), static_cast<float>(new_position.y));
            mouse_world_position = offset + (pos - center) / zoom;
        }
    };

    State state;

    ViewportHandler(sf::Vector2f size)
        : state(size)
    {
        state.updateState();
    }

    void addOffset(sf::Vector2f v)
    {
        state.offset += v / state.zoom;
        state.updateState();
    }

    void zoom(float f)
    {
        state.zoom *= f;
        state.updateState();
    }

    void wheelZoom(float w)
    {
        if (w) {
            const float zoom_amount = 1.2f;
            const float delta = w > 0 ? zoom_amount : 1.0f / zoom_amount;
            zoom(delta);
        }
    }

    void reset()
    {
        state.zoom = 1.0f;
        setFocus(state.center);
    }

    const sf::Transform& getTransform() const
    {
        return state.transform;
    }

    void click(sf::Vector2f relative_click_position)
    {
        state.mouse_position = relative_click_position;
        state.clicking = true;
    }

    void unclick()
    {
        state.clicking = false;
    }

    void setMousePosition(sf::Vector2f new_mouse_position)
    {
        if (state.clicking) {
            addOffset(state.mouse_position - new_mouse_position);
        }
        state.updateMousePosition(new_mouse_position);
    }

    void setFocus(sf::Vector2f focus_position)
    {
        state.offset = focus_position;
        state.updateState();
    }

    void setZoom(float zoom)
    {
        state.zoom = zoom;
        state.updateState();
    }

    sf::Vector2f getMouseWorldPosition() const
    {
        return state.mouse_world_position;
    }

    sf::Vector2f getScreenCoords(sf::Vector2f world_pos) const
    {
        return state.transform.transformPoint(world_pos);
    }
};
