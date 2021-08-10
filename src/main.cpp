#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include <fstream>

#include "viewport_handler.hpp"
#include "number_generator.hpp"
#include "event_manager.hpp"
#include "simulation.hpp"


int WIN_WIDTH = 1920;
int WIN_HEIGHT = 1080;
const uint32_t max_history = 100;

const sf::Vector3f stable_color(0, 255, 0);


float dot(const sf::Vector2f& v1, const sf::Vector2f& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

float length(const sf::Vector2f& v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalize(const sf::Vector2f& v)
{
	return v / length(v);
}

struct Conf
{
	uint32_t win_width = 1920;
	uint32_t win_height = 1080;
	uint32_t objects_count = 20;
	uint32_t min_size = 5;
	uint32_t max_size = 70;
};


Conf loadUserConf()
{
	Conf conf;
	std::ifstream conf_file("conf.txt");
	if (conf_file) {
		conf_file >> conf.win_width;
		conf_file >> conf.win_height;
		conf_file >> conf.objects_count;
		conf_file >> conf.min_size;
		conf_file >> conf.max_size;
	}
	else {
		std::cout << "Couldn't find 'conf.txt', loading default" << std::endl;
	}

	return conf;
}


void createVpHandlerControls(sfev::EventManager& ev_manager, ViewportHandler& vp_handler)
{
    // Viewport Handler controls
    ev_manager.addMousePressedCallback(sf::Mouse::Left, [&](sfev::CstEv) {
        vp_handler.click(ev_manager.getFloatMousePosition());
    });

    ev_manager.addMouseReleasedCallback(sf::Mouse::Left, [&](sfev::CstEv) {
        vp_handler.unclick();
    });

    ev_manager.addEventCallback(sf::Event::MouseMoved, [&](sfev::CstEv) {
        vp_handler.setMousePosition(ev_manager.getFloatMousePosition());
    });
    
    ev_manager.addKeyPressedCallback(sf::Keyboard::R, [&](sfev::CstEv) {
        vp_handler.reset();
    });
    
    ev_manager.addEventCallback(sf::Event::MouseWheelScrolled, [&](sfev::CstEv e) {
        vp_handler.wheelZoom(e.mouseWheelScroll.delta);
    });
}


int main()
{
	Conf conf = loadUserConf();
	WIN_WIDTH = conf.win_width;
	WIN_HEIGHT = conf.win_height;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(conf.win_width, conf.win_height), "NoCol", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    float speedDownFactor = 1;
    float speedDownCounter = 1;
    float waitingSpeedFactor = 1;
    float speedDownFactorGoal = 1;
    int iterations = 0;

    bool drawTraces = true;
    bool synccEnable = true;

    int nBalls = conf.objects_count;
    int maxSize = conf.max_size;
    int minSize = conf.min_size;

	const float spawn_range_factor = 0.5f;
    std::vector<Ball> balls;
	for (int i(0); i < nBalls; i++) {
		const float angle = RNGf::getUnder(2.0f * 3.141592653f);
		const float radius = 350.0f;

		const float start_x = radius * cos(angle);
		const float start_y = radius * sin(angle);

		balls.push_back(Ball(start_x + WIN_WIDTH * 0.5f, start_y + WIN_HEIGHT * 0.5f,
			RNGf::getRange(static_cast<float>(minSize), static_cast<float>(maxSize))));
	}

    sf::RenderTexture traces, blurTexture, renderer;
    blurTexture.create(WIN_WIDTH, WIN_HEIGHT);
    traces.create(WIN_WIDTH, WIN_HEIGHT);
    renderer.create(WIN_WIDTH, WIN_HEIGHT);

    traces.clear(sf::Color::Black);
    traces.display();

	ViewportHandler vp_handler(sf::Vector2f(WIN_WIDTH, WIN_HEIGHT));
    sfev::EventManager ev_manager(window, true);
    
	ev_manager.addKeyReleasedCallback(sf::Keyboard::A, [&](sfev::CstEv) { drawTraces = !drawTraces; });
	ev_manager.addKeyReleasedCallback(sf::Keyboard::C, [&](sfev::CstEv) { traces.clear(); });
	ev_manager.addKeyReleasedCallback(sf::Keyboard::Space, [&](sfev::CstEv) {
		speedDownFactorGoal = speedDownFactor == 1 ? 10.0f : 1.0f;
	});
	ev_manager.addKeyReleasedCallback(sf::Keyboard::Escape, [&](sfev::CstEv) {window.close(); });
	ev_manager.addKeyReleasedCallback(sf::Keyboard::E, [&](sfev::CstEv) {
		synccEnable = !synccEnable;
		window.setVerticalSyncEnabled(synccEnable);
	});
    
    createVpHandlerControls(ev_manager, vp_handler);

	const Ball* focus = nullptr;

	uint32_t ok_count = 0;
 
    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		ev_manager.processEvents();
		const sf::RenderStates rs = vp_handler.getRenderState();

		if (waitingSpeedFactor != speedDownFactorGoal) {
			waitingSpeedFactor += speedDownFactorGoal - waitingSpeedFactor;
		}

		bool stable = true;
		if (!speedDownCounter) {
			for (Ball& ball : balls) {
				ball.stable = true;
				ball.save();
			}

			stable = update(balls, 1);
			if (!stable && ok_count < 200) {
				ok_count = 0;
			}

			if (stable) {
				++ok_count;
			}

			if (waitingSpeedFactor) {
				speedDownFactor = waitingSpeedFactor;
			}
			speedDownCounter = speedDownFactor;
		}

		updatePos(balls, speedDownFactor, speedDownCounter);

        window.clear(sf::Color::Black);

		sf::RenderStates rs_traces = rs;
		rs_traces.blendMode = sf::BlendAdd;
		for (const Ball& b : balls) {
			if (drawTraces) {
				sf::VertexArray trace = b.getVA();
				window.draw(trace, rs_traces);
			}
		}

		sf::Vector2f center_of_mass(0.0f, 0.0f);
        for (const Ball& b : balls)
        {
			const sf::Vector3f unstable_color(255, 0, 0);
			const float stable_ratio = (ok_count > 199) ? 1.0f : std::min(1.0f, b.stableCount / 255.0f);
			const sf::Vector3f vec_color = stable_ratio * stable_color + (1.0f - stable_ratio) * unstable_color;

            int c = b.stableCount > 255 ? 255 : b.stableCount;
			sf::Color color(static_cast<sf::Uint8>(vec_color.x), static_cast<sf::Uint8>(vec_color.y), static_cast<sf::Uint8>(vec_color.z));
            float r = b.r;

            if (speedDownFactor > 1)
                r = b.r;

			center_of_mass += b.position;

            sf::CircleShape ballRepresentation(r);
			ballRepresentation.setPointCount(64);
            ballRepresentation.setFillColor(color);
            ballRepresentation.setOrigin(r, r);
            ballRepresentation.setPosition(b.position);
			window.draw(ballRepresentation, rs);
        }
		center_of_mass /= float(balls.size());

		const float com_r = 4.0f;

		window.display();

        iterations++;
    }

    return 0;
}
