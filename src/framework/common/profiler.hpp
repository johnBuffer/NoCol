#pragma once
#include <cstdint>
#include <SFML/System.hpp>


struct Profiler
{
	struct Element
	{
		uint64_t start, total;
		Element()
			: start(0)
			, total(0)
		{}

		void reset()
		{
			start = 0;
			total = 0;
		}

		float asMilliseconds() const
		{
			return total * 0.001f;
		}
	};

	sf::Clock clock;

	Profiler()
	{
		clock.restart();
	}

	void start(Element& elem)
	{
		elem.start = clock.getElapsedTime().asMicroseconds();
	}

	void stop(Element& elem)
	{
		elem.total += clock.getElapsedTime().asMicroseconds() - elem.start;
	}
};
