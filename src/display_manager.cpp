#include "display_manager.hpp"


DisplayManager::DisplayManager(sf::RenderWindow & window)
	: DisplayManager(window, window)
{
}

DisplayManager::DisplayManager(sf::RenderTarget& target, sf::RenderWindow& window)
	: m_window(window)
	, event_manager(window)
	, m_target(target)
	, m_zoom(1.0f)
	, m_offsetX(0.0f)
	, m_offsetY(0.0f)
	, speed_mode(false)
	, m_va(sf::Quads, 0)
	, update(true)
	, debug_mode(false)
	, clic(false)
	, m_mouse_button_pressed(false)
	, pause(false)
	, draw_markers(true)
{
	m_windowOffsetX = m_window.getSize().x * 0.5f;
    m_windowOffsetY = m_window.getSize().y * 0.5f;

	m_offsetX = m_windowOffsetX;
	m_offsetY = m_windowOffsetY;

	event_manager.addEventCallback(sf::Event::Closed, [&](sfev::CstEv ev) {window.close(); });

	event_manager.addKeyReleasedCallback(sf::Keyboard::R, [&](sfev::CstEv) {
		m_offsetX = 0.0f;
		m_offsetY = 0.0f;
		m_zoom = 1.0f;
	});

	event_manager.addEventCallback(sf::Event::MouseWheelMoved, [&](sfev::CstEv ev) {zoom(1 + ev.mouseWheel.delta * 0.2f); });

	event_manager.addEventCallback(sf::Event::MouseButtonPressed, [&](sfev::CstEv ev) {
		if (ev.mouseButton.button == sf::Mouse::Left) {
			m_mouse_button_pressed = true;
			m_drag_clic_position = m_mouse_position;
			m_clic_position = m_mouse_position;
		}
		else if (ev.mouseButton.button == sf::Mouse::Right) {
			clic = true;
		}
	});

	event_manager.addEventCallback(sf::Event::MouseButtonReleased, [&](sfev::CstEv ev) {
		m_mouse_button_pressed = false;
		if (ev.mouseButton.button == sf::Mouse::Right) {
			clic = false;
		}
	});

	event_manager.addEventCallback(sf::Event::MouseMoved, [&](sfev::CstEv ev) {
		if (m_mouse_button_pressed) // in this case we are dragging
		{
			// updating displayManager offset
			const float vx = float(m_mouse_position.x - m_drag_clic_position.x);
			const float vy = float(m_mouse_position.y - m_drag_clic_position.y);
			addOffset(vx, vy);
			m_drag_clic_position = m_mouse_position;
		}
	});
}

sf::Vector2f DisplayManager::worldCoordToDisplayCoord(const sf::Vector2f& worldCoord) const
{
    float worldX = worldCoord.x;
    float worldY = worldCoord.y;

    float viewCoordX = (worldX-m_offsetX)*m_zoom+m_windowOffsetX;
    float viewCoordY = (worldY-m_offsetY)*m_zoom+m_windowOffsetY;

    return sf::Vector2f(viewCoordX, viewCoordY);
}

sf::Vector2f DisplayManager::displayCoordToWorldCoord(const sf::Vector2f& viewCoord) const
{
    float viewCoordX = viewCoord.x;
    float viewCoordY = viewCoord.y;

    float worldCoordX = (viewCoordX-m_windowOffsetX)/m_zoom+m_offsetX;
    float worldCoordY = (viewCoordY-m_windowOffsetY)/m_zoom+m_offsetY;

    return sf::Vector2f(worldCoordX, worldCoordY);
}

sf::RenderStates DisplayManager::getRenderStates() const
{
	sf::RenderStates rs;
	rs.transform.translate(m_windowOffsetX, m_windowOffsetY);
	rs.transform.scale(m_zoom, m_zoom);
	rs.transform.translate(-m_offsetX, -m_offsetY);

	return rs;
}

std::vector<sf::Event> DisplayManager::processEvents()
{
	m_mouse_position = sf::Mouse::getPosition(m_window);
	return event_manager.processEvents();
}

