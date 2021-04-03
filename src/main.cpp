#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include <fstream>

#include "display_manager.hpp"
#include "number_generator.hpp"


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


struct Ball
{
	sf::Vector2f position, velocity;
	float r;

	std::vector<sf::Vector2f> position_history;
	uint32_t current_idx;

    bool stable;
    int stableCount;

	Ball()
		: position_history(max_history)
		, current_idx(0)
	{
	}

	Ball(float x, float y, float arg_r)
		: position(x, y)
		, velocity(static_cast<float>(rand() % 8 - 4), static_cast<float>(rand() % 8 - 4))
		, r(arg_r)
		, position_history(max_history)
		, current_idx(0)
    {
		for (sf::Vector2f& pos : position_history) {
			pos = sf::Vector2f(x, y);
		}
        stableCount = 0;
    }

    void save()
    {
		position_history[current_idx] = position;
		current_idx = (++current_idx) % max_history;
    }

	sf::VertexArray getVA() const
	{
		sf::VertexArray va(sf::LineStrip, max_history);
		for (uint32_t i(0); i < max_history; ++i) {
			const uint32_t actual_idx = (i + current_idx) % max_history;
			const float ratio = i / float(max_history);
			va[i].position = position_history[actual_idx];
			const sf::Vector3f color = ratio * stable_color;
			va[i].color = sf::Color(static_cast<sf::Uint8>(color.x), static_cast<sf::Uint8>(color.y), static_cast<sf::Uint8>(color.z));
		}

		return va;
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

const Ball* getBallAt(const sf::Vector2f& position, std::vector<Ball>& balls)
{
	for (const Ball& ball : balls) {
		const sf::Vector2f v = position - ball.position;
		const float dist = sqrt(v.x*v.x + v.y * v.y);
		if (dist < ball.r) {
			return &ball;
		}
	}

	return nullptr;
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


int main()
{
	Conf conf = loadUserConf();
	WIN_WIDTH = conf.win_width;
	WIN_HEIGHT = conf.win_height;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(conf.win_width, conf.win_height), "NoCol", sf::Style::Fullscreen, settings);
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

	DisplayManager display_manager(window);
	display_manager.event_manager.addKeyReleasedCallback(sf::Keyboard::A, [&](sfev::CstEv) { drawTraces = !drawTraces; });
	display_manager.event_manager.addKeyReleasedCallback(sf::Keyboard::C, [&](sfev::CstEv) { traces.clear(); });
	display_manager.event_manager.addKeyReleasedCallback(sf::Keyboard::Space, [&](sfev::CstEv) {  
		speedDownFactorGoal = speedDownFactor == 1 ? 10.0f : 1.0f;
	});
	display_manager.event_manager.addKeyReleasedCallback(sf::Keyboard::Escape, [&](sfev::CstEv) {window.close(); });
	display_manager.event_manager.addKeyReleasedCallback(sf::Keyboard::E, [&](sfev::CstEv) { 
		synccEnable = !synccEnable;
		window.setVerticalSyncEnabled(synccEnable);
	});

	const Ball* focus = nullptr;

	uint32_t ok_count = 0;
 
    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		std::vector<sf::Event> events = display_manager.processEvents();
		const sf::RenderStates rs = display_manager.getRenderStates();

		if (waitingSpeedFactor != speedDownFactorGoal) {
			waitingSpeedFactor += speedDownFactorGoal - waitingSpeedFactor;
		}

		if (display_manager.clic) {
			focus = getBallAt(display_manager.getWorldMousePosition(), balls);
			display_manager.clic = false;
		}

		if (focus) {
			display_manager.setOffset(focus->position.x, focus->position.y);
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
		/*sf::CircleShape com_representation(com_r);
		com_representation.setFillColor(sf::Color::Cyan);
		com_representation.setOrigin(com_r, com_r);
		com_representation.setPosition(center_of_mass);
		window.draw(com_representation, rs);*/

		window.display();

        iterations++;
    }

    return 0;
}
