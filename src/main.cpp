#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include <fstream>

#include "DynamicBlur.h"
#include "display_manager.hpp"


int WIN_WIDTH = 1600;
int WIN_HEIGHT = 900;

struct Ball
{
    double x, y, oldX, oldY;
    double vx, vy;
    double r;

    bool stable;
    int stableCount;

    Ball(double arg_x, double arg_y, double arg_r) : x(arg_x), y(arg_y), vx(5), vy(-1), r(arg_r)
    {
        stableCount = 0;
    }

    void save()
    {
        oldX = x;
        oldY = y;
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
		currentBall.vx += (WIN_WIDTH/2-currentBall.x) * dt;
		currentBall.vy += (WIN_HEIGHT/2-currentBall.y) * dt;

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
    for (Ball& currentBall : balls)
    {
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
    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "TEST", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    double speedDownFactor = 1;
    double speedDownCounter = 1;
    double waitingSpeedFactor = 1;
    double speedDownFactorGoal = 1;
    int iterations = 0;

    bool drawTraces = true;
    bool synccEnable = true;

    int nBalls = 20;
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
            for (int i(0); i<nBalls; i++)
            {
                balls[i].stable = true;
                balls[i].save();
            }

            stable = update(balls, 1);
            if (waitingSpeedFactor)
            {
                speedDownFactor = waitingSpeedFactor;
            }
            speedDownCounter = speedDownFactor;
        }

        updatePos(balls, speedDownFactor, speedDownCounter);

        renderer.clear(sf::Color::Black);

		if (drawTraces)
		{
			renderer.draw(sf::Sprite(traces.getTexture()), rs);
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

			if (drawTraces)
			{
				sf::VertexArray trace(sf::Lines, 2);
				trace[0].position = sf::Vector2f(b.x, b.y);
				trace[1].position = sf::Vector2f(b.oldX, b.oldY);

				trace[0].color = color;
				trace[1].color = color;

				traces.draw(trace);
			}

            /*if (!b.stableCount && speedDownCounter == speedDownFactor-1)
            {
                double r = b.r;
                ballRepresentation.setRadius(r);
                ballRepresentation.setOrigin(r, r);
                ballRepresentation.setFillColor(sf::Color::Red);
                traces.draw(ballRepresentation);

                r -= 4;

                ballRepresentation.setRadius(r);
                ballRepresentation.setOrigin(r, r);
                ballRepresentation.setFillColor(sf::Color::Black);
                traces.draw(ballRepresentation);

                r += 2;

                sf::VertexArray circle(sf::LinesStrip, 20);
                for (int i(0); i<20; i++)
                {
                    double pi = 3.14159;
                    double x = b.x + r*sin(2*pi/19.0*i);
                    double y = b.y + r*cos(2*pi/19.0*i);
                    circle[i].position = sf::Vector2f(x, y);
                }

                traces.draw(circle);
            }*/
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
