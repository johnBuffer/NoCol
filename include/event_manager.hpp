#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <functional>
#include <vector>


namespace sfev
{


// Helper using for shorter types
using EventCallback = std::function<void(const sf::Event& event)>;

template<typename T>
using EventCallbackMap = std::unordered_map<T, EventCallback>;

using CstEv = const sf::Event&;


/*
	This class handles subtyped events like keyboard or mouse events
	The unpack function allows to get relevant information from the processed event
*/
template<typename T>
class SubTypeManager
{
public:
	SubTypeManager(std::function<T(const sf::Event&)> unpack) :
		m_unpack(unpack)
	{}

	~SubTypeManager() = default;

	void processEvent(const sf::Event& event) const
	{
		T sub_value = m_unpack(event);
		auto it(m_callmap.find(sub_value));
		if (it != m_callmap.end())
		{
			// Call its associated callback
			(it->second)(event);
		}
	}

	void addCallback(const T& sub_value, EventCallback callback)
	{
		m_callmap[sub_value] = callback;
	}

private:
	EventCallbackMap<T> m_callmap;
	std::function<T(const sf::Event&)> m_unpack;
};

/*
	This class handles any type of event and call its associated callbacks if any.
	To process key event in a more convenient way its using a KeyManager
*/
class EventManager
{
public:
	EventManager(sf::Window& window) :
		m_window(window),
		m_key_pressed_manager([](const sf::Event& event) {return event.key.code; }),
		m_key_released_manager([](const sf::Event& event) {return event.key.code; }),
		m_mouse_pressed_manager([](const sf::Event& event) {return event.mouseButton.button; }),
		m_mouse_released_manager([](const sf::Event& event) {return event.mouseButton.button; })
	{
		// Register key events built in callbacks
		this->addEventCallback(sf::Event::EventType::KeyPressed, [&](const sf::Event& event) {m_key_pressed_manager.processEvent(event); });
		this->addEventCallback(sf::Event::EventType::KeyReleased, [&](const sf::Event& event) {m_key_released_manager.processEvent(event); });
		this->addEventCallback(sf::Event::EventType::MouseButtonPressed, [&](const sf::Event& event) {m_mouse_pressed_manager.processEvent(event); });
		this->addEventCallback(sf::Event::EventType::MouseButtonReleased, [&](const sf::Event& event) {m_mouse_released_manager.processEvent(event); });
	}

	// Attach new callback to an event
	void addEventCallback(sf::Event::EventType type, EventCallback callback)
	{
		m_events_callmap[type] = callback;
	}

	// Calls events' attached callbacks
	std::vector<sf::Event> processEvents() const
	{
		// TODO process events from vector
		std::vector<sf::Event> non_processed;
		// Iterate over events
		sf::Event event;
		while (m_window.pollEvent(event)) {
			// If event type is registred
			sf::Event::EventType type = event.type;
			auto it(m_events_callmap.find(type));
			if (it != m_events_callmap.end()) {
				// Call its associated callback
				(it->second)(event);
			}
			else {
				non_processed.push_back(event);
			}
		}

		return non_processed;
	}

	// Removes a callback
	void removeCallback(sf::Event::EventType type)
	{
		// If event type is registred
		auto it(m_events_callmap.find(type));
		if (it != m_events_callmap.end()) {
			// Remove its associated callback
			m_events_callmap.erase(it);
		}
	}

	// Adds a key pressed callback
	void addKeyPressedCallback(sf::Keyboard::Key key_code, EventCallback callback)
	{
		m_key_pressed_manager.addCallback(key_code, callback);
	}

	// Adds a key released callback
	void addKeyReleasedCallback(sf::Keyboard::Key key_code, EventCallback callback)
	{
		m_key_released_manager.addCallback(key_code, callback);
	}

	// Adds a mouse pressed callback
	void addMousePressedCallback(sf::Mouse::Button button, EventCallback callback)
	{
		m_mouse_pressed_manager.addCallback(button, callback);
	}

	// Adds a mouse released callback
	void addMouseReleasedCallback(sf::Mouse::Button button, EventCallback callback)
	{
		m_mouse_released_manager.addCallback(button, callback);
	}

private:
	sf::Window& m_window;

	SubTypeManager<sf::Keyboard::Key> m_key_pressed_manager;
	SubTypeManager<sf::Keyboard::Key> m_key_released_manager;

	SubTypeManager<sf::Mouse::Button> m_mouse_pressed_manager;
	SubTypeManager<sf::Mouse::Button> m_mouse_released_manager;

	EventCallbackMap<sf::Event::EventType> m_events_callmap;
};

} // End namespace