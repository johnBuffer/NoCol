#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include <fstream>

#include "DynamicBlur.h"
#include "display_manager.hpp"


int WIN_WIDTH = 1920;
int WIN_HEIGHT = 1080;
const uint32_t max_history = 100;





struct Ball
{
	sf::Vector2f position, velocity;
	double r;

	std::vector<sf::Vector2f> position_history;
	uint32_t current_idx;

    bool stable;
    int stableCount;

	Ball()
		: position_history(max_history)
		, current_idx(0)
	{}

	Ball(double x, double y, double arg_r)
		: position(x, y)
		, velocity(5.0f, -1.0f)
		, r(arg_r)
		, position_history(max_history)
		, current_idx(0)
    {
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

    int nBalls = balls.size();

	const float dt = 0.008f;

	const sf::Vector2f center_position(WIN_WIDTH * 0.5f, WIN_HEIGHT * 0.5f);

    for (int i(0); i<nBalls; i++)
	{
	    Ball& currentBall = balls[i];
		// Attraction to center
		currentBall.velocity += 0.2f * (center_position - currentBall.position) * dt;

		for (int k=i+1; k<nBalls; k++) {
		    Ball& collider = balls[k];
			const sf::Vector2f collide_vec = currentBall.position - collider.position;
			const float dist = sqrt(collide_vec.x*collide_vec.x + collide_vec.y*collide_vec.y);

			const float minDist = currentBall.r+collider.r;

			if (dist < minDist && dist) {
			    stable = false;

			    currentBall.stable = false;
			    collider.stable = false;

				const sf::Vector2f collide_axe = collide_vec / dist;

				currentBall.position += (minDist-dist) * collide_vec;

				collider.position -= (minDist - dist) * collide_vec;

				/*const float collider_vx = collider.vx;
				const float collider_vy = collider.vy;

				collider.vx += currentBall.vx;
				collider.vy += currentBall.vy;

				currentBall.vx += collider_vx;
				currentBall.vy += collider_vy;*/
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

const Ball* getBallAt(const sf::Vector2f& position, const std::vector<Ball>& balls)
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


int main()
{
    srand(time(0));
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

    int nBalls = 10;
    int maxSize = 100;
    int minSize = 2;

    std::ifstream infile;
    infile.open("config");
    std::cout << "Loading config" << std::endl;
    infile >> nBalls;
    infile >> maxSize;
    infile >> minSize;

	const float spawn_range_factor = 0.8f;
    std::vector<Ball> balls;
    for (int i(0); i<nBalls; i++)
        balls.push_back(Ball(rand()%WIN_WIDTH*spawn_range_factor + WIN_WIDTH * spawn_range_factor * 0.5f * (1.0f - spawn_range_factor), 
			                 rand()%WIN_HEIGHT*spawn_range_factor + WIN_HEIGHT * spawn_range_factor * 0.5f * (1.0f - spawn_range_factor), rand()%(maxSize-minSize)+minSize));

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
 
    while (window.isOpen())
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		std::vector<sf::Event> events = display_manager.processEvents();
		const sf::RenderStates rs = display_manager.getRenderStates();

        if (iterations%5 == 0 && waitingSpeedFactor != speedDownFactorGoal)
        {
            waitingSpeedFactor += speedDownFactorGoal-waitingSpeedFactor;
        }

		if (display_manager.clic) {
			focus = getBallAt(display_manager.getWorldMousePosition(), balls);
			display_manager.clic = false;
		}

		if (focus) {
			display_manager.setOffset(focus->position.x, focus->position.y);
		}

        bool stable = true;
        if (!speedDownCounter)
        {
            int nBalls = balls.size();
            for (Ball& ball : balls) {
                ball.stable = true;
                ball.save();
            }

            stable = update(balls, 1);
            if (waitingSpeedFactor) {
                speedDownFactor = waitingSpeedFactor;
            }
            speedDownCounter = speedDownFactor;
        }

        updatePos(balls, speedDownFactor, speedDownCounter);

        renderer.clear(sf::Color::Black);

		sf::RenderStates rs_traces = rs;
		rs_traces.blendMode = sf::BlendAdd;
		for (const Ball& b : balls) {
			if (drawTraces) {
				sf::VertexArray trace = b.getVA();
				renderer.draw(trace, rs_traces);
			}
		}

        for (const Ball& b : balls)
        {
            int c = b.stableCount > 255 ? 255 : b.stableCount;
            sf::Color color = sf::Color(255 - c, c, 0);
            double r = b.r;

            if (speedDownFactor > 1)
                r = b.r;

            sf::CircleShape ballRepresentation(r);
			ballRepresentation.setPointCount(128);
            ballRepresentation.setFillColor(color);
            ballRepresentation.setOrigin(r, r);
            ballRepresentation.setPosition(b.position.x, b.position.y);
            renderer.draw(ballRepresentation, rs);
        }

        sf::CircleShape stableIndicator(10);
        stableIndicator.setPosition(10, 10);
        stableIndicator.setFillColor(sf::Color::Red);
        if (stable)
            stableIndicator.setFillColor(sf::Color::Green);

        renderer.draw(stableIndicator);
        renderer.display();

        window.draw(sf::Sprite(renderer.getTexture()));
        window.display();

        iterations++;
    }

    return 0;
}
