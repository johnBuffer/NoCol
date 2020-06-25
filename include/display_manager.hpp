#pragma once

#include <SFML/Graphics.hpp>
#include "event_manager.hpp"


class DisplayManager
{
public:
	DisplayManager(sf::RenderWindow& window);
    DisplayManager(sf::RenderTarget& target, sf::RenderWindow& window);

    //offset mutators
    void setOffset(float x, float y) {m_offsetX=x; m_offsetY=y;};
    void setOffset(const sf::Vector2f& off) {m_offsetX=off.x; m_offsetY=off.y;};

    void addOffset(float x, float y) {m_offsetX-=x/m_zoom; m_offsetY-=y/m_zoom;};
    void addOffset(const sf::Vector2f& off) {m_offsetX-=off.x/m_zoom; m_offsetY-=off.y/m_zoom;};

    // set the absolute zoom
    void setZoom(float zoom) {m_zoom = zoom;};

    // zoom
    void zoom(float zoomFactor) {m_zoom *= zoomFactor;};

    // draw the current world
	sf::RenderStates getRenderStates() const;

	std::vector<sf::Event> processEvents();

    // getters
    sf::Vector2f getOffset() const {return sf::Vector2f(m_offsetX, m_offsetY);};
    float getZoom() const {return m_zoom;};
	sf::Vector2f worldCoordToDisplayCoord(const sf::Vector2f&) const;
	sf::Vector2f displayCoordToWorldCoord(const sf::Vector2f&) const;

	bool clic;
	bool pause;
	bool draw_markers;
	bool update;
	float render_time;
	bool speed_mode;
	bool debug_mode;

	sf::Vector2f getClicPosition() const
	{
		return sf::Vector2f(static_cast<float>(m_clic_position.x), static_cast<float>(m_clic_position.y));
	}

	sf::Vector2f getWorldMousePosition() const
	{
		return displayCoordToWorldCoord(sf::Vector2f(static_cast<float>(m_mouse_position.x), static_cast<float>(m_mouse_position.y)));
	}

	sfev::EventManager event_manager;

private:
	sf::RenderTarget& m_target;
	sf::RenderWindow& m_window;
    sf::Texture m_texture;
	sf::VertexArray m_va;

	bool m_mouse_button_pressed;
	sf::Vector2i m_drag_clic_position, m_clic_position, m_mouse_position;

    float m_zoom, m_offsetX, m_offsetY, m_windowOffsetX, m_windowOffsetY;
};
