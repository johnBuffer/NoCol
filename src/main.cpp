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
    double x, y, oldX, oldY;
    double vx, vy;
    double r;

	std::vector<sf::Vector2f> position_history;
	uint32_t current_idx;

    bool stable;
    int stableCount;

	Ball()
		: position_history(max_history)
		, current_idx(0)
	{}

	Ball(double arg_x, double arg_y, double arg_r)
		: x(arg_x)
		, y(arg_y)
		, vx(5)
		, vy(-1)
		, r(arg_r)
		, position_history(max_history)
		, current_idx(0)
    {
        stableCount = 0;
    }

    void save()
    {
        oldX = x;
        oldY = y;

		position_history[current_idx] = sf::Vector2f(x, y);
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

    for (int i(0); i<nBalls; i++)
	{
	    Ball& currentBall = balls[i];
		// Attraction to center
		currentBall.vx += 0.2f * (WIN_WIDTH/2-currentBall.x) * dt;
		currentBall.vy += 0.2f * (WIN_HEIGHT/2-currentBall.y) * dt;

		for (int k=0; k<nBalls; k++)
		{
		    if (k == i)
                break;

		    Ball& collider = balls[k];
			double dx = currentBall.x-collider.x;
			double dy = currentBall.y-collider.y;
			double dist = sqrt(dx*dx+dy*dy);

			double minDist = currentBall.r+collider.r;

			if (dist < minDist && dist)
			{
			    stable = false;

			    currentBall.stable = false;
			    collider.stable = false;

				double rx = dx/dist;
				double ry = dy/dist;

				currentBall.x += (minDist-dist)*rx;
				currentBall.y += (minDist-dist)*ry;

				collider.x -= (minDist-dist)*rx;
				collider.y -= (minDist-dist)*ry;
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

void updatePos(std::vector<Ball>& balls, double& speedDownFactor, double& speedDownCounter)
{
	const float dt = 0.016f;
    for (Ball& currentBall : balls) {
        currentBall.x += dt * currentBall.vx/speedDownFactor;
        currentBall.y += dt * currentBall.vy/speedDownFactor;
    }

    speedDownCounter--;
}

const Ball* getBallAt(const sf::Vector2f& position, const std::vector<Ball>& balls)
{
	for (const Ball& ball : balls) {
		const float vx = position.x - ball.x;
		const float vy = position.y - ball.y;
		const float dist = sqrt(vx*vx + vy * vy);
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

    int nBalls = 80;
    int maxSize = 42;
    int minSize = 2;

    std::ifstream infile;
    infile.open("config");
    std::cout << "Loading config" << std::endl;
    infile >> nBalls;
    infile >> maxSize;
    infile >> minSize;

    std::vector<Ball> balls;
    for (int i(0); i<nBalls; i++)
        balls.push_back(Ball(rand()%WIN_WIDTH, rand()%WIN_HEIGHT, rand()%(maxSize-minSize)+minSize));

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
			display_manager.setOffset(focus->x, focus->y);
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
            ballRepresentation.setFillColor(color);
            ballRepresentation.setOrigin(r, r);
            ballRepresentation.setPosition(b.x, b.y);
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
