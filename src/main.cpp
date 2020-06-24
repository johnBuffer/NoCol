#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include <fstream>

#include "display_manager.hpp"
#include <SFML/Audio.hpp>


int WIN_WIDTH = 1920;
int WIN_HEIGHT = 1080;
const uint32_t max_history = 100;


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

	bool close_to_selected;
	float last_dist_to_selected;
	sf::Sound sound;

	Ball()
		: position_history(max_history)
		, current_idx(0)
	{
	}

	Ball(double x, double y, double arg_r)
		: position(x, y)
		, velocity(rand() % 8 - 4, rand() % 8 - 4)
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
			va[i].color = sf::Color(0, 255 * ratio, 0);
		}

		return va;
	}
};

bool update(std::vector<Ball>& balls, double speed)
{
    bool stable = true;

    const uint32_t nBalls = balls.size();
	const float dt = 0.008f;
	const float attraction_force = 5000.0f;
	const float attraction_force_bug = 0.02f;
	const sf::Vector2f center_position(WIN_WIDTH * 0.5f, WIN_HEIGHT * 0.5f);
	const float attractor_threshold = 50.0f;

    for (uint32_t i(0); i<nBalls; i++) {
	    Ball& current_ball = balls[i];
		// Attraction to center
		const sf::Vector2f to_center = center_position - current_ball.position;
		const float dist_to_center = length(to_center);
		current_ball.velocity += attraction_force_bug * to_center;

		for (int k=i+1; k<nBalls; k++) {
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

	for (int i(0); i<nBalls; i++)
    {
        if(balls[i].stable)
            balls[i].stableCount++;
        else
            balls[i].stableCount = 0;
    }

	return stable;
}

void updatePos(std::vector<Ball>& balls, float speedDownFactor, double& speedDownCounter)
{
	const float dt = 0.016f;
    for (Ball& currentBall : balls) {
       currentBall.position += (dt / speedDownFactor) * currentBall.velocity;
    }

    speedDownCounter--;
}

void resetOtherThan(const Ball* selected, std::vector<Ball>& balls)
{
	for (Ball& ball : balls) {
		if (&ball != selected) {
			ball.close_to_selected = false;
			ball.last_dist_to_selected = -1.0f;
		}
	}
}

const Ball* getBallAt(const sf::Vector2f& position, std::vector<Ball>& balls)
{
	for (const Ball& ball : balls) {
		const sf::Vector2f v = position - ball.position;
		const float dist = sqrt(v.x*v.x + v.y * v.y);
		if (dist < ball.r) {
			resetOtherThan(&ball, balls);
			return &ball;
		}
	}

	return nullptr;
}


int main()
{
	const uint64_t seed = time(0);
    srand(seed);
	std::cout << seed << std::endl;
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "NoCol", sf::Style::Fullscreen, settings);
    window.setVerticalSyncEnabled(true);

    double speedDownFactor = 1;
    double speedDownCounter = 1;
    double waitingSpeedFactor = 1;
    double speedDownFactorGoal = 1;
    int iterations = 0;

    bool drawTraces = true;
    bool synccEnable = true;

    int nBalls = 5;
    int maxSize = 50;
    int minSize = 20;

    std::ifstream infile;
    infile.open("config");
    std::cout << "Loading config" << std::endl;
    infile >> nBalls;
    infile >> maxSize;
    infile >> minSize;

	sf::SoundBuffer buffer;
	buffer.loadFromFile("../sounds/move_2.wav");
	//buffer.loadFromFile("../sounds/move_deep.wav");

	const float spawn_range_factor = 0.5f;
    std::vector<Ball> balls;
	for (int i(0); i < nBalls; i++) {
		balls.push_back(Ball((rand() % WIN_WIDTH)*spawn_range_factor + WIN_WIDTH * 0.5f * (1.0f - spawn_range_factor),
			(rand() % WIN_HEIGHT)*spawn_range_factor + WIN_HEIGHT * 0.5f * (1.0f - spawn_range_factor),
			rand() % (maxSize - minSize) + minSize));

		balls.back().sound.setBuffer(buffer);
		balls.back().sound.setLoop(true);
	}

	/*for (int i(0); i < 1; i++) {
		balls.push_back(Ball(rand() % WIN_WIDTH*spawn_range_factor + WIN_WIDTH * spawn_range_factor * 0.5f * (1.0f - spawn_range_factor),
			rand() % WIN_HEIGHT*spawn_range_factor + WIN_HEIGHT * spawn_range_factor * 0.5f * (1.0f - spawn_range_factor),
			10));

		balls.back().sound.setBuffer(buffer);
		balls.back().sound.setLoop(true);
	}*/

	const float close_threshold = 50.0f;

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
		speedDownFactorGoal = speedDownFactor == 1 ? 10 : 1;
	});
	display_manager.event_manager.addKeyReleasedCallback(sf::Keyboard::Escape, [&](sfev::CstEv) {window.close(); });
	display_manager.event_manager.addKeyReleasedCallback(sf::Keyboard::E, [&](sfev::CstEv) { 
		synccEnable = !synccEnable;
		window.setVerticalSyncEnabled(synccEnable);
	});

	const Ball* focus = nullptr;

	uint32_t ok_count = 0;

	sf::Clock clock;
 
    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		std::vector<sf::Event> events = display_manager.processEvents();
		const sf::RenderStates rs = display_manager.getRenderStates();

		if (clock.getElapsedTime().asSeconds() > 10.0f)
		{

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
				int nBalls = balls.size();
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

			if (focus) {
				for (Ball& b : balls) {
					if (&b == focus) {
						continue;
					}

					const float dist_to_selected = length(b.position - focus->position);

					if (dist_to_selected - b.r < close_threshold) {
						if (b.last_dist_to_selected != -1.0f) {
							const float proximity_speed = b.last_dist_to_selected / dist_to_selected;
							if (!b.close_to_selected) {
								b.close_to_selected = true;
								b.sound.play();
							}

							const float speed_factor = std::pow(proximity_speed, 2.0f);
							const float min_radius = focus->r + b.r;
							const float volume = std::min(80.0f, 20.0f * (1.0f - dist_to_selected / (close_threshold + b.r)));
							b.sound.setVolume(speed_factor * volume);
							b.sound.setPitch(speed_factor + focus->r / float(b.r));
						}
					}
					else {
						if (b.close_to_selected) {
							b.sound.pause();
						}
						b.close_to_selected = false;
					}

					b.last_dist_to_selected = dist_to_selected;
				}
			}
		}

        window.clear(sf::Color::Black);

		sf::RenderStates rs_traces = rs;
		rs_traces.blendMode = sf::BlendAdd;
		for (const Ball& b : balls) {
			if (drawTraces) {
				sf::VertexArray trace = b.getVA();
				window.draw(trace, rs_traces);
			}
		}

        for (const Ball& b : balls)
        {
            int c = b.stableCount > 255 ? 255 : b.stableCount;
            sf::Color color = ok_count >= 200 ? sf::Color::Green : sf::Color(255 - c, c, 0);
            double r = b.r;

            if (speedDownFactor > 1)
                r = b.r;

            sf::CircleShape ballRepresentation(r);
			ballRepresentation.setPointCount(128);
            ballRepresentation.setFillColor(color);
            ballRepresentation.setOrigin(r, r);
            ballRepresentation.setPosition(b.position.x, b.position.y);
			window.draw(ballRepresentation, rs);
        }

		window.display();

        iterations++;
    }

    return 0;
}
